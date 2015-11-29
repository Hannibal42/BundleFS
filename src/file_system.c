#include "file_system.h"
#include "buffer.h"

bool free_disk_space(struct FILE_SYSTEM *fs, uint size);
bool resize_inode_block(struct FILE_SYSTEM *fs);
void load_all_tab(uint8_t *buffer, struct FILE_SYSTEM *fs);
void load_ino_tab(uint8_t *buffer, struct FILE_SYSTEM *fs);

enum FSRESULT fs_mkfs(struct disk *disk)
{
	uint i;
	struct FILE_SYSTEM *fs;
	unsigned long sec_cnt, sec_size, fil_cnt, size;
	struct AT_WINDOW *window;

	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sec_size) != RES_OK)
		return FS_ERROR;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sec_cnt) != RES_OK)
		return FS_ERROR;

	fs = malloc(sizeof(struct FILE_SYSTEM));

	/* Defines the start value of inodes */
	fil_cnt = INODE_CNT;
	if (fil_cnt < 8)
		fil_cnt = 8;

	/*The number of sectors needed for the allocation table*/
	fs->alloc_table_size = div_up(sec_cnt, sec_size * 8);
	fs->alloc_table = 1;
	fs->alloc_table_buffer_size = AT_BUFFER_SIZE;
	/*Make inode_alloc_table*/
	fs->inode_alloc_table_size = fs->alloc_table_size;
	fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;
	fs->inode_sec = sec_size / sizeof(struct INODE);
	/*This becomes 0 if fil_cnt < fs->inode_sec*/
	fs->inode_max = fil_cnt - (fil_cnt % fs->inode_sec);
	if (fs->inode_max == 0)
		fs->inode_max = fs->inode_sec;
	fs->inode_block_size = fs->inode_max / fs->inode_sec;
	fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
	fs->sector_size = sec_size;
	fs->sector_count = sec_cnt;

	/* Make Inode table */
	size = sec_size * fs->inode_alloc_table_size;
	for (i = 0; i < size; ++i)
		IT_BUFFER[i] = 0xFF;

	delete_seq(IT_BUFFER, 0, fs->inode_max);

	if (disk_write(disk, IT_BUFFER, fs->inode_alloc_table,
	fs->inode_alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;

	/* Make Allocation table */
	size = sec_size * fs->alloc_table_size;
	for (i = 0; i < size; ++i)
		SEC_BUFFER[i] = 0xFF;

	for (i = 0; i < fs->alloc_table_size; ++i)
		if (disk_write(disk, SEC_BUFFER, fs->alloc_table + i, 1) != RES_OK)
			return FS_DISK_ERROR;

	window = malloc(sizeof(struct AT_WINDOW));
	fs->disk = disk;
	init_window(window, fs, AT_BUFFER);
	fs->at_win = window;

	size = sec_cnt - (fs->inode_block + fs->inode_block_size);
	if (delete_seq_global(fs->at_win, 0, size))
		return FS_DISK_ERROR;

	/*Write superblock*/
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	if (disk_write(disk, SEC_BUFFER, 0, 1) != RES_OK)
		return FS_DISK_ERROR;

	free(window);
	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number,
	struct INODE *file)
{
	uint offset, sector;
	uint8_t tmp;

	if (disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;

	offset = number % 8;
	sector = number / 8;

	tmp =  0x80 >> offset;
	if (!(IT_BUFFER[sector] & tmp))
		return FS_PARAM_ERROR;

	offset = (number % fs->inode_sec) * sizeof(struct INODE);
	sector = number / fs->inode_sec;

	if (disk_read(fs->disk, SEC_BUFFER, fs->inode_block + sector, 1)
		!= RES_OK)
		return FS_DISK_ERROR;

	memcpy(file, &SEC_BUFFER[offset], sizeof(struct INODE));

	return FS_OK;
}

enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file)
{
	uint bit_length, tmp, sec_size;

	sec_size = fs->sector_size - check_size();
	bit_length = div_up(file->size, sec_size);
	tmp = fs->sector_count - (file->location + bit_length);

	/* Free bits in the allocation table */
	if (disk_read(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;
	delete_seq(AT_BUFFER, tmp, bit_length);

	/*Free bit in inode_table*/
	if (disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;
	write_bit(IT_BUFFER, file->inode_offset, false);

	if (disk_write(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;
	if (disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;

	return FS_OK;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	uint8_t *buffer, uint length)
{
	uint i, k, fil_sec, pad, sec_size;
	uint32_t tmp;

	if (length > file->size)
		return FS_PARAM_ERROR;

	sec_size = fs->sector_size - check_size();
	fil_sec = div_up(file->size, sec_size);
	pad = file->size % sec_size;

	for (i = 0; i < (fil_sec - 1); ++i) {
		if (disk_read(fs->disk, SEC_BUFFER, file->location + i, 1)
			!= RES_OK)
			return FS_DISK_ERROR;
		for (k = 0; k < sec_size; ++k)
			buffer[i * sec_size + k] = SEC_BUFFER[k];
		/* Checks the crc sum */
		tmp = con8to32(&SEC_BUFFER[sec_size]);
		if (tmp != calc_crc32_8(SEC_BUFFER, sec_size))
			return FS_CHECK_ERROR;
	}

	/* Last sector, the padding needs to be used here */
	if (disk_read(fs->disk, SEC_BUFFER, file->location + i, 1) != RES_OK)
		return FS_DISK_ERROR;

	for (k = 0; k < sec_size; ++k)
		if (k < pad)
			buffer[i * sec_size + k] = SEC_BUFFER[k];
	/* Checks the crc sum */
	tmp = con8to32(&SEC_BUFFER[sec_size]);
	if (tmp != calc_crc32_8(SEC_BUFFER, sec_size))
		return FS_CHECK_ERROR;

	return FS_OK;
}

enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	uint8_t *buffer)
{
	uint i, k, fil_sec, pad, sec_size;
	uint32_t tmp;

	sec_size = fs->sector_size - check_size();
	fil_sec = div_up(file->size, sec_size);
	pad = file->size % sec_size;

	for (i = 0; i < (fil_sec - 1); ++i) {
		for (k = 0; k < sec_size; ++k)
			SEC_BUFFER[k] = buffer[i * sec_size + k];
		/* Adds the checksum to the end of the block */
		tmp = calc_crc32_8(SEC_BUFFER, sec_size);
		con32to8(&SEC_BUFFER[sec_size], tmp);
		if (disk_write(fs->disk, SEC_BUFFER,
			file->location + i, 1) != RES_OK)
			return FS_DISK_ERROR;
	}

	/* Last sector, the padding needs to be used here */
	for (k = 0; k < sec_size; ++k) {
		if (k < pad)
			SEC_BUFFER[k] = buffer[i * sec_size + k];
		else
			SEC_BUFFER[k] = 0xFF;
	}
	/* Adds the checksum to the end of the block */
	tmp = calc_crc32_8(SEC_BUFFER, sec_size);
	con32to8(&SEC_BUFFER[sec_size], tmp);
	if (disk_write(fs->disk, SEC_BUFFER, file->location + i, 1)
		!= RES_OK)
		return FS_DISK_ERROR;

	return FS_OK;
}

enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file)
{
	write_inode(fs, file);
	return FS_OK;
}

enum FSRESULT fs_mount(struct disk *disk, struct FILE_SYSTEM *fs)
{

	unsigned long sector_size;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	if (disk_read(disk, SEC_BUFFER, 0, 1) != RES_OK)
		return FS_DISK_ERROR;

	memcpy(fs, SEC_BUFFER, sizeof(struct FILE_SYSTEM));
	fs->disk = disk;
	fs->at_win = malloc(sizeof(struct AT_WINDOW));
	init_window(fs->at_win, fs, AT_BUFFER);

	return FS_OK;
}

unsigned long fs_getfree(struct FILE_SYSTEM *fs)
{
	unsigned long tmp, byte_cnt;
	uint i;

	byte_cnt = div_up(fs->sector_count, 8);

	load_all_tab(AT_BUFFER, fs);

	tmp = 0;
	for (i = 0; i < byte_cnt; ++i)
		tmp += (8 - popcount((uint8_t) AT_BUFFER[i]));

	return tmp * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
uint32_t size, uint64_t time_to_live, bool custody)
{
	int all_off, ino_off;
	uint bit_cnt, byt_ino_tab, byt_all_tab, sec_size;

	sec_size = fs->sector_size - check_size();
	bit_cnt = div_up(size, sec_size);

	load_all_tab(AT_BUFFER, fs);
	load_ino_tab(IT_BUFFER, fs);

	/*Finds and writes a free inode in the inode alloc table*/
	byt_ino_tab = fs->inode_alloc_table_size * fs->sector_size;
	ino_off = find_bit(IT_BUFFER, byt_ino_tab);
	if (ino_off < 0) {
		if (resize_inode_block(fs)) {
			if (disk_read(fs->disk, IT_BUFFER,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size) != RES_OK)
				return FS_DISK_ERROR;
			/* This mount is needed because the metadata changed */
			fs_mount(fs->disk, fs);
			++byt_ino_tab;
			ino_off = find_bit(IT_BUFFER, byt_ino_tab);
		} else {
			return FS_FULL;
		}
	}

	/*Finds a sequence of bits that are empty in the allocation table*/
	byt_all_tab  = fs->alloc_table_size * fs->sector_size;
	all_off = find_seq(AT_BUFFER, byt_all_tab, bit_cnt);

	if (all_off < 0) {
		if (free_disk_space(fs, size))
			if (disk_read(fs->disk, AT_BUFFER, fs->alloc_table,
				fs->alloc_table_size) != RES_OK)
				return FS_DISK_ERROR;
			all_off = find_seq(AT_BUFFER, byt_all_tab,
				bit_cnt);
			if (disk_read(fs->disk, IT_BUFFER,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size) != RES_OK)
				return FS_DISK_ERROR;
			ino_off = find_bit(IT_BUFFER, byt_ino_tab);

		if (all_off < 0)
			return FS_FULL;
	}

	/* Write the buffer */
	write_seq(AT_BUFFER, all_off, bit_cnt);
	write_bit(IT_BUFFER, ino_off, true);

	file->size = size;
	file->location = fs->sector_count - all_off - bit_cnt;
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = ino_off;

	/* Write all to disk */
	if (disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;
	if (disk_write(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return FS_DISK_ERROR;
	write_inode(fs, file);

	return FS_OK;
}

/* TODO: Remove */
void load_all_tab(uint8_t *buffer, struct FILE_SYSTEM *fs)
{
	uint32_t i;

	for (i = 0; i < fs->alloc_table_size; ++i) {
		disk_read(fs->disk, SEC_BUFFER,
			fs->alloc_table + i, 1);
		memcpy(buffer, SEC_BUFFER, fs->sector_size);
	}
}

/* TODO: Remove */
void load_ino_tab(uint8_t *buffer, struct FILE_SYSTEM *fs)
{
	uint32_t i;

	for (i = 0; i < fs->inode_alloc_table_size; ++i) {
		disk_read(fs->disk, SEC_BUFFER,
			fs->inode_alloc_table + i, 1);
		memcpy(buffer, SEC_BUFFER, fs->sector_size);
	}
}

bool free_disk_space(struct FILE_SYSTEM *fs, uint size)
{
	struct INODE *inode;

	inode = malloc(sizeof(struct INODE));

	if (!find_ino_length(fs, inode, size)) {
		free(inode);
		return false;
	}
	/* TODO: Delete more than one inode if needed */

	fs_delete(fs, inode);

	/* TODO: Notify upcn that a Budnle was deleted */

	free(inode);
	return true;
}

bool resize_inode_block(struct FILE_SYSTEM *fs)
{
	uint tmp;

	if (disk_read(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return false;

	if (disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return false;

	tmp = fs->sector_count - (fs->inode_block + fs->inode_block_size);
	tmp -= 1;

	if (!check_seq(AT_BUFFER, tmp, 1))
		return false;

	write_seq(AT_BUFFER, tmp, 1);
	delete_seq(IT_BUFFER, fs->inode_max, fs->inode_sec);

	if (disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return false;

	if (disk_write(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size) != RES_OK)
		return false;

	/* Update superblock */
	fs->inode_block_size += 1;
	fs->inode_max += fs->inode_sec;
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	if (disk_write(fs->disk, SEC_BUFFER, 0, 1) != RES_OK)
		return false;

	return true;
}
