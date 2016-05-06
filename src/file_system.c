#include "file_system.h"

bool free_disk_space(struct FILE_SYSTEM *fs, uint size);
bool resize_inode_block(struct FILE_SYSTEM *fs);

enum FSRESULT fs_mkfs(struct DISK *disk, uint sector_size)
{
	uint i;
	struct FILE_SYSTEM *fs;
	unsigned long sec_cnt, sec_size, fil_cnt, size;
	struct AT_WINDOW *at_win;
	struct AT_WINDOW *it_win;

	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sec_size) != RES_OK)
		return FS_ERROR;

	disk->sector_block_mapping = sector_size / sec_size;
	sec_size = sector_size;

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
	fs->inode_alloc_table_buffer_size = IT_BUFFER_SIZE;
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
		SEC_BUFFER[i] = 0xFF;
	for (i = 0; i < fs->inode_alloc_table_size; ++i)
		if (disk_write(disk, SEC_BUFFER, fs->inode_alloc_table + i, 1)
				!= RES_OK)
			return FS_DISK_ERROR;

	it_win = malloc(sizeof(struct AT_WINDOW));
	fs->disk = disk;
	init_window_it(it_win, fs, IT_BUFFER);
	fs->it_win = it_win;

	if (!delete_seq_global(it_win, 0, fs->inode_max))
		return FS_DISK_ERROR;
	if (!save_window(it_win))
		return FS_DISK_ERROR;


	/* Make Allocation table */
	size = sec_size * fs->alloc_table_size;
	for (i = 0; i < size; ++i)
		SEC_BUFFER[i] = 0xFF;

	for (i = 0; i < fs->alloc_table_size; ++i)
		if (disk_write(disk, SEC_BUFFER, fs->alloc_table + i, 1)
				!= RES_OK)
			return FS_DISK_ERROR;

	at_win = malloc(sizeof(struct AT_WINDOW));
	init_window(at_win, fs, AT_BUFFER);
	fs->at_win = at_win;

	size = sec_cnt - (fs->inode_block + fs->inode_block_size);
	if (!delete_seq_global(fs->at_win, 0, size))
		return FS_DISK_ERROR;

	if (!save_window(fs->at_win))
		return FS_DISK_ERROR;

	/*Write superblock*/
	for (i = 0; i < fs->sector_size; ++i)
		SEC_BUFFER[i] = 0x00;
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	if (disk_write(disk, SEC_BUFFER, 0, 1) != RES_OK)
		return FS_DISK_ERROR;

	free(at_win);
	free(it_win);
	fs->it_win = NULL;
	fs->at_win = NULL;
	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number,
	struct INODE *file)
{
	uint offset, sector;
	uint8_t tmp;

	offset = number % 8;
	sector = number / 8;

	if (!move_window(fs->it_win, sector /
			fs->inode_alloc_table_buffer_size))
		return FS_DISK_ERROR;

	sector %= fs->inode_alloc_table_buffer_size;

	tmp =  0x80 >> offset;
	if (!(fs->it_win->buffer[sector] & tmp))
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
	if (!delete_seq_global(fs->at_win, tmp, bit_length))
		return FS_DISK_ERROR;

	/*Free bit in inode_table*/
	if (!delete_seq_global(fs->it_win, file->inode_offset, 1))
		return FS_DISK_ERROR;

	if (!save_window(fs->it_win))
		return FS_DISK_ERROR;
	if (!save_window(fs->at_win))
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

enum FSRESULT fs_mount(struct DISK *disk, struct FILE_SYSTEM *fs)
{

	unsigned long sector_size;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	if (disk_read(disk, SEC_BUFFER, 0, 1) != RES_OK)
		return FS_DISK_ERROR;

	memcpy(fs, SEC_BUFFER, sizeof(struct FILE_SYSTEM));
	fs->disk = disk;
	fs->at_win = &AT_WINDOW;
	fs->it_win = &IT_WINDOW;
	init_window(fs->at_win, fs, AT_BUFFER);
	init_window_it(fs->it_win, fs, IT_BUFFER);

	return FS_OK;
}

unsigned long fs_getfree(struct FILE_SYSTEM *fs)
{
	unsigned long tmp, byte_cnt;
	uint i;

	byte_cnt = div_up(fs->sector_count, 8);

	if (disk_read(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size) != RES_OK)
		return 0;

	tmp = 0;
	for (i = 0; i < byte_cnt; ++i)
		tmp += (8 - popcount((uint8_t) AT_BUFFER[i]));

	return tmp * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
uint32_t size, uint64_t time_to_live, bool custody)
{

	uint bit_cnt, byt_ino_tab, sec_size, all_off, ino_off;

	sec_size = fs->sector_size - check_size();
	bit_cnt = div_up(size, sec_size);

	/*Finds and writes a free inode in the inode alloc table*/
	byt_ino_tab = fs->inode_alloc_table_size * fs->sector_size;
	if (!find_seq_global(fs->it_win, 1, &ino_off)) {
		if (resize_inode_block(fs)) {
			if (!reload_window(fs->it_win))
				return FS_DISK_ERROR;
			/* This mount is needed because the metadata changed */
			fs_mount(fs->disk, fs);
			++byt_ino_tab;
			if (!find_seq_global(fs->it_win, 1, &ino_off))
				return FS_DISK_ERROR;
		} else {
			return FS_FULL;
		}
	}

	/*Finds a sequence of bits that are empty in the allocation table*/
	if (!find_seq_global(fs->at_win, bit_cnt, &all_off)) {
		if (free_disk_space(fs, size))
			if (!reload_window(fs->at_win))
				return FS_DISK_ERROR;
			if (!find_seq_global(fs->at_win, bit_cnt, &all_off))
				return FS_FULL;
			if (!reload_window(fs->it_win))
				return FS_DISK_ERROR;
			if (!find_seq_global(fs->it_win, 1, &ino_off))
				return FS_FULL;

	}

	/* Write the buffer */
	if (!write_seq_global(fs->at_win, all_off, bit_cnt))
		return FS_DISK_ERROR;
	if (!write_seq_global(fs->it_win, ino_off, 1))
		return FS_DISK_ERROR;

	file->size = size;
	file->location = fs->sector_count - all_off - bit_cnt;
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = ino_off;

	/* Write all to disk */
	if (!save_window(fs->at_win))
		return FS_DISK_ERROR;
	if (!save_window(fs->it_win))
		return FS_DISK_ERROR;
	write_inode(fs, file);

	return FS_OK;
}


enum FSRESULT fs_create_write(struct FILE_SYSTEM *fs, struct INODE *file,
	uint8_t *buffer, uint32_t size, uint64_t time_to_live, bool custody)
{

	uint bit_cnt, byt_ino_tab, sec_size, all_off, ino_off;

	sec_size = fs->sector_size - check_size();
	bit_cnt = div_up(size, sec_size);

	/*Finds and writes a free inode in the inode alloc table*/
	byt_ino_tab = fs->inode_alloc_table_size * fs->sector_size;
	if (!find_seq_global(fs->it_win, 1, &ino_off)) {
		if (resize_inode_block(fs)) {
			if (!reload_window(fs->it_win))
				return FS_DISK_ERROR;
			/* This mount is needed because the metadata changed */
			fs_mount(fs->disk, fs);
			++byt_ino_tab;
			if (!find_seq_global(fs->it_win, 1, &ino_off))
				return FS_DISK_ERROR;
		} else {
			return FS_FULL;
		}
	}

	/*Finds a sequence of bits that are empty in the allocation table*/
	if (!find_seq_global(fs->at_win, bit_cnt, &all_off)) {
		if (free_disk_space(fs, size))
			if (!reload_window(fs->at_win))
				return FS_DISK_ERROR;
			if (!find_seq_global(fs->at_win, bit_cnt, &all_off))
				return FS_FULL;
			if (!reload_window(fs->it_win))
				return FS_DISK_ERROR;
			if (!find_seq_global(fs->it_win, 1, &ino_off))
				return FS_FULL;

	}

	/* Write the buffer */
	if (!write_seq_global(fs->at_win, all_off, bit_cnt))
		return FS_DISK_ERROR;
	if (!write_seq_global(fs->it_win, ino_off, 1))
		return FS_DISK_ERROR;

	file->size = size;
	file->location = fs->sector_count - all_off - bit_cnt;
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = ino_off;

	/* Write all to disk */
	if (!save_window(fs->at_win))
		return FS_DISK_ERROR;
	if (!save_window(fs->it_win))
		return FS_DISK_ERROR;
	if (fs_write(fs, file, buffer) != FS_OK)
		return FS_ERROR;
	write_inode(fs, file);

	return FS_OK;
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

	/* TODO: Notify upcn that a bundle was deleted */

	free(inode);
	return true;
}

bool resize_inode_block(struct FILE_SYSTEM *fs)
{
	uint tmp;

	if (!reload_window(fs->at_win))
		return FS_DISK_ERROR;

	tmp = fs->sector_count - (fs->inode_block + fs->inode_block_size);
	tmp -= 1;

	if (!check_seq_global(fs->at_win, tmp, 1))
		return false;

	if (!write_seq_global(fs->at_win, tmp, 1))
		return false;
	if (!delete_seq_global(fs->it_win, fs->inode_max, fs->inode_sec))
		return false;

	if (!save_window(fs->at_win))
		return false;

	if (!save_window(fs->it_win))
		return false;

	/* Update superblock */
	fs->inode_block_size += 1;
	fs->inode_max += fs->inode_sec;
	memcpy(SEC_BUFFER, fs, sizeof(struct FILE_SYSTEM));
	if (disk_write(fs->disk, SEC_BUFFER, 0, 1) != RES_OK)
		return false;

	return true;
}
