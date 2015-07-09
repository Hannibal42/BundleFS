#include "file_system.h"
#include "buffer.h"

bool free_disk_space(struct FILE_SYSTEM *fs, uint size);
bool resize_inode_block(struct FILE_SYSTEM *fs);
void load_all_tab(uint8_t *buffer,struct FILE_SYSTEM *fs);
void load_ino_tab(uint8_t *buffer, struct FILE_SYSTEM *fs);

enum FSRESULT fs_mkfs(struct disk *disk)
{
	uint i, tmp;
	struct FILE_SYSTEM *fs;
	unsigned long sec_cnt, sec_size, fil_cnt, size, dif;

	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sec_size) != RES_OK)
		return FS_ERROR;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sec_cnt) != RES_OK)
		return FS_ERROR;

	fs = malloc(sizeof(struct FILE_SYSTEM));

	/*TODO:Change this to a better thing*/
	fil_cnt = sec_cnt / 8;
	if (fil_cnt < 8)
		fil_cnt = 8;

	/*The number of sectors needed for the allocation table*/
	fs->alloc_table_size = div_up(sec_cnt, sec_size * 8);
	fs->alloc_table = 1;
	size = sec_size * fs->alloc_table_size;
	/*Calculates the padding of the sector*/
	dif = div_up(sec_cnt, 8) % sec_size;
	dif += (fs->alloc_table_size - 1) * sec_size;
	if (sec_cnt % 8 != 0)
		dif = dif - 1;
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			AT_BUFFER[i] = 0xFF;
		else
			AT_BUFFER[i] = 0x00;
	}

	disk_write(disk, (char *) AT_BUFFER, fs->alloc_table, fs->alloc_table_size);

	/*Make inode_alloc_table*/
	fs->inode_alloc_table_size = div_up(fil_cnt, sec_size * 8);
	fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;

	size = sec_size * fs->inode_alloc_table_size;

	dif = div_up(fil_cnt, 8) % sec_size;
	dif += (fs->inode_alloc_table_size - 1) * sec_size;
	if (sec_cnt % 8 != 0)
		dif = dif - 1;
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			IT_BUFFER[i] = 0xFF;
		else
			IT_BUFFER[i] = 0x00;
	}
	disk_write(disk, (char *) IT_BUFFER, fs->inode_alloc_table,
	fs->inode_alloc_table_size);

	/*Make superblock*/
	fs->inode_sec = sec_size / sizeof(struct INODE);
	fs->inode_max = fil_cnt - (fil_cnt % fs->inode_sec); /*TODO: This is 0 if fil_cnt < inode_sec */
	fs->inode_block_size = fs->inode_max / fs->inode_sec;
	fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
	fs->sector_size = sec_size;
	fs->sector_count = sec_cnt;

	/*Write superblock*/
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	disk_write(disk, (char *) SEC_BUFFER, 0, 1);

	/* Update alloc_table padding */
	disk_read(disk, (char *) AT_BUFFER, fs->alloc_table, fs->alloc_table_size);
	tmp = sec_cnt;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq(AT_BUFFER, tmp, fs->inode_block + fs->inode_block_size);
	disk_write(disk, (char *) AT_BUFFER, fs->alloc_table, fs->alloc_table_size);

	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number,
	struct INODE *file)
{
	uint i, k, *ino_pos, ino_cnt, offset;
	struct INODE *inodes;

	disk_read(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	offset = 0;
	ino_pos = malloc(fs->inode_sec * sizeof(uint));
	inodes = (struct INODE*) INO_BUFFER;

	/*Loads a block with inodes, extracts the valid ones and 
	compares their id to number*/
	for (i = 0; i < fs->inode_block_size; ++i) {
		get_ino_pos(fs, IT_BUFFER, offset, ino_pos, &ino_cnt);
		load_inode_block(fs, inodes, ino_pos, ino_cnt, i);
		for (k = 0; k < ino_cnt; ++k)
			if (inodes[k].id == number)
				goto END;
		offset += fs->inode_sec;
	}
	END:

	if (i >= fs->inode_block_size) {
		free(ino_pos);
		return FS_ERROR;
	}

	memcpy(file, &inodes[k], sizeof(struct INODE));

	free(ino_pos);
	return FS_OK;
}

enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file)
{
	uint bit_length, tmp, sec_size;

	sec_size = fs->sector_size - check_size();
	bit_length = div_up(file->size, sec_size);
	tmp = fs->sector_count - (file->location + bit_length);

	/* Free bits in the allocation table */
	disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);
	delete_seq(AT_BUFFER, tmp, bit_length);

	/*Free bit in inode_table*/
	disk_read(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	write_bit(IT_BUFFER, file->inode_offset, false);

	disk_write(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	return FS_OK;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer, uint length)
{
	uint i, k, fil_sec, pad, sec_size;

	if (length > file->size)
		return FS_PARAM_ERROR;

	sec_size = fs->sector_size - check_size();
	fil_sec = div_up(file->size, sec_size);
	pad = file->size % sec_size;

	for (i = 0; i < (fil_sec - 1); ++i) {
		reset_fake_crc();
		disk_read(fs->disk, (char *) SEC_BUFFER, file->location + i, 1);
		for (k = 0; k < sec_size; ++k) {
			buffer[i * sec_size + k] = SEC_BUFFER[k];
			calc_fake_crc(buffer[i * sec_size + k]);
		}
		/* Checks the crc sum */
		if (SEC_BUFFER[k] != calc_fake_crc(0x00))
			return FS_CHECK_ERROR;
	}

	/* Last sector, the padding needs to be used here */
	disk_read(fs->disk, (char *) SEC_BUFFER, file->location + i, 1);
	reset_fake_crc();
	for (k = 0; k < sec_size; ++k) {
		if (k < pad) {
			buffer[i * sec_size + k] = SEC_BUFFER[k];
			calc_fake_crc(buffer[i * sec_size + k]);
		} else {
			calc_fake_crc(0xFF);
		}
	}
	/* Checks the crc sum */
	if (SEC_BUFFER[k] != calc_fake_crc(0x00))
		return FS_CHECK_ERROR;

	return FS_OK;
}

enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	uint i, k, fil_sec, pad, sec_size;

	sec_size = fs->sector_size - check_size();
	fil_sec = div_up(file->size, sec_size);
	pad = file->size % sec_size;

	for (i = 0; i < (fil_sec - 1); ++i) {
		reset_fake_crc();
		for (k = 0; k < sec_size; ++k) {
			calc_fake_crc(buffer[i * sec_size + k]);
			SEC_BUFFER[k] = buffer[i * sec_size + k];
		}
		/* Adds the checksum to the end of the block */
		SEC_BUFFER[k] = calc_fake_crc(0x00);
		disk_write(fs->disk, (char *) SEC_BUFFER, file->location + i, 1);
	}

	/* Last sector, the padding needs to be used here */
	reset_fake_crc();
	for (k = 0; k < sec_size; ++k) {
		if (k < pad) {
			calc_fake_crc(buffer[i * sec_size + k]);
			SEC_BUFFER[k] = buffer[i * sec_size + k];
		} else {
			calc_fake_crc(0xFF);
			SEC_BUFFER[k] = 0xFF;
		}
	}
	/* Adds the checksum to the end of the block */
	SEC_BUFFER[k] = calc_fake_crc(0x00);
	disk_write(fs->disk, (char *) SEC_BUFFER, file->location + i, 1);

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

	disk_read(disk, (char *) SEC_BUFFER, 0, 1);

	memcpy(fs, SEC_BUFFER, sizeof(struct FILE_SYSTEM));
	fs->disk = disk; /*TODO: Remove this in the working system */

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
unsigned long size, uint time_to_live, bool custody)
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
			disk_read(fs->disk, (char *) IT_BUFFER,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size);
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
			disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
				fs->alloc_table_size);
			all_off = find_seq(AT_BUFFER, byt_all_tab,
				bit_cnt);
			disk_read(fs->disk, (char *) IT_BUFFER,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size);
			ino_off = find_bit(IT_BUFFER, byt_ino_tab);

		if (all_off < 0) {
			return FS_FULL;
		}
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
	disk_write(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);
	write_inode(fs, file);

	return FS_OK;
}

void load_all_tab(uint8_t *buffer, struct FILE_SYSTEM *fs)
{
	uint32_t i;

	for (i = 0; i < fs->alloc_table_size; ++i) {
		disk_read(fs->disk, (char *) SEC_BUFFER, fs->alloc_table + i, 1);
		memcpy(buffer, SEC_BUFFER, fs->sector_size);
	}
}

void load_ino_tab(uint8_t *buffer, struct FILE_SYSTEM *fs)
{
	uint32_t i;

	for (i = 0; i < fs->inode_alloc_table_size; ++i) {
		disk_read(fs->disk, (char *) SEC_BUFFER, fs->inode_alloc_table + i, 1);
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

	disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	disk_read(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	tmp = fs->sector_count - (fs->inode_block + fs->inode_block_size);
	tmp -= 1;

	if (!check_seq(AT_BUFFER, tmp, 1)) {
		return false;
	}

	write_seq(AT_BUFFER, tmp, 1);
	delete_seq(IT_BUFFER, fs->inode_max, fs->inode_sec);

	disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	disk_write(fs->disk, (char *) IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	/* Update superblock */
	fs->inode_block_size += 1;
	fs->inode_max += fs->inode_sec;
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	disk_write(fs->disk, (char *) SEC_BUFFER, 0, 1);

	return true;
}
