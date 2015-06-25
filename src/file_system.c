#include "file_system.h"
#include "buffer.h"

bool free_disk_space(struct FILE_SYSTEM *fs, uint size);
bool resize_inode_block(struct FILE_SYSTEM *fs);

enum FSRESULT fs_mkfs(struct disk *disk)
{
	/*Check if the drive is ready*/
	uint i, tmp;
	struct FILE_SYSTEM *fs;/*the file system to be created*/
	unsigned long sector_count, sector_size, max_file_count, size,
	dif, super_size;/*Size of the superblock in sectors*/
	char *buffer; /*Used to make the fs superblock*/

	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK)
		return FS_ERROR;

	fs = (struct FILE_SYSTEM *) malloc(sizeof(struct FILE_SYSTEM));
	super_size = div_up(sizeof(struct FILE_SYSTEM), sector_size);

	/*TODO:Change this to a better thing*/
	max_file_count = sector_count / 8;
	if (max_file_count < 8)
		max_file_count = 8;

	/*The number of sectors needed for the allocation table*/
	fs->alloc_table_size = div_up(sector_count, sector_size * 8);
	fs->alloc_table = super_size; /*first comes the superblock*/
	size = sector_size * fs->alloc_table_size;
	/*Calculates the padding of the sector*/
	dif = div_up(sector_count, 8) % sector_size;
	dif += (fs->alloc_table_size - 1) * sector_size;
	if (sector_count % 8 != 0)
		dif = dif - 1;
	buffer = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			buffer[i] = 0xFF;
		else
			buffer[i] = 0x00;
	}

	disk_write(disk, buffer, fs->alloc_table, fs->alloc_table_size);
	free(buffer);

	/*Make inode_alloc_table*/
	fs->inode_alloc_table_size = div_up(max_file_count,
		sector_size * 8);
	fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;

	size = sector_size * fs->inode_alloc_table_size;

	dif = div_up(max_file_count, 8) % sector_size;
	dif += (fs->inode_alloc_table_size - 1) * sector_size;
	if (sector_count % 8 != 0)
		dif = dif - 1;
	buffer = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			buffer[i] = 0xFF;
		else
			buffer[i] = 0x00;
	}
	disk_write(disk, buffer, fs->inode_alloc_table,
	fs->inode_alloc_table_size);
	free(buffer);

	/*Make Inode Block*/
	fs->inode_sec = sector_size / sizeof(struct INODE);
	fs->inode_max = max_file_count;
	fs->inode_block_size = div_up(fs->inode_max, fs->inode_sec);
	fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
	fs->sector_size = sector_size;
	fs->sector_count = sector_count;

	/*Make superblock*/
	buffer = malloc(fs->sector_size * super_size);
	memcpy(buffer, fs, sizeof(struct FILE_SYSTEM));
	disk_write(disk, buffer, 0, super_size);
	free(buffer);

	/* Update alloc_table padding */
	buffer = malloc(fs->alloc_table_size * fs->sector_size);
	disk_read(disk, buffer, fs->alloc_table, fs->alloc_table_size);
	tmp = sector_count;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq((uint8_t *)buffer, tmp, fs->inode_block +
		fs->inode_block_size);
	disk_write(disk, buffer, fs->alloc_table, fs->alloc_table_size);

	free(buffer);
	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number,
	struct INODE *file)
{
	uint i, ino_cnt;
	struct INODE *inodes;

	ino_cnt = inodes_used(fs);
	inodes = malloc(ino_cnt * sizeof(struct INODE));
	load_inodes(fs, inodes);

	for (i = 0; i < ino_cnt; ++i) {
		if (inodes[i].id == number)
			break;
	}

	if (i >= ino_cnt) {
		free(inodes);
		return FS_ERROR;
	}

	memcpy(file, &inodes[i], sizeof(struct INODE));

	free(inodes);
	return FS_OK;
}

enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file)
{
	uint i, bit_length, tmp;
	uint8_t *all_tab, *ino_tab, *buffer;

	/* Free bits in the allocation table */
	all_tab = malloc(fs->sector_size * fs->alloc_table_size);
	disk_read(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);
	bit_length = div_up(file->size, fs->sector_size);
	//TODO: calculate the new checksize;
	tmp = fs->sector_count - (file->location + bit_length);
	delete_seq(all_tab, tmp, bit_length);

	/*Free bit in inode_table*/
	ino_tab = malloc(fs->sector_size * fs->inode_alloc_table_size);
	disk_read(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	write_bit(ino_tab, file->inode_offset, false);

	/* Delete Inode */
	buffer = malloc(fs->sector_size);
	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x00;

	disk_write(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);
	disk_write(fs->disk, (char *) buffer, fs->inode_block +
		file->inode_offset, 1);

	free(all_tab);
	free(ino_tab);
	free(buffer);
	return FS_OK;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer, uint length)
{
	uint i, sector_count, check_count;
	uint8_t *tmp;

	if (length > file->size)
		return FS_PARAM_ERROR;

	sector_count = div_up(file->size, fs->sector_size);
	disk_read(fs->disk, buffer, file->location, sector_count - 1);
	disk_read(fs->disk, (char *) SEC_BUFFER, file->location + sector_count - 1, 1);

	memcpy(buffer, SEC_BUFFER, file->size % fs->sector_size);

	//TODO: Read with new check_size;
	//check_count = div_up(file->check_size, fs->sector_size);
	//
	//tmp = malloc(check_count);
	//disk_read(fs->disk, (char *) tmp, file->location + sector_count, check_count);
	//if (!checksum_check(buffer, tmp, file, fs->sector_size)) {
	//	free(tmp);
	//	return FS_CHECK_ERROR;
	//}

	//free(tmp);
	return FS_OK;
}

enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	uint fil_sec, che_sec, pad;
	uint8_t *che_buf;
	char *sec_buf;

	fil_sec = div_up(file->size, fs->sector_size);
	pad = file->size % fs->sector_size;
	//TODO: Rebuild
	/*che_sec = div_up(file->check_size, fs->sector_size);

	//che_buf = malloc(che_sec * fs->sector_size);
	//checksum((uint8_t *) buffer, file->size, che_buf, file->check_size);
	*/
	/* Makes a padding if the file does not fit the sectors */ /*
	if (pad) {
		sec_buf = malloc(fs->sector_size);
		memcpy(sec_buf, &buffer[file->size - pad], pad);
		disk_write(fs->disk, buffer, file->location, fil_sec - 1);
		disk_write(fs->disk, sec_buf, file->location + fil_sec - 1, 1);
		free(sec_buf);
	} else {
		disk_write(fs->disk, buffer, file->location, fil_sec);
	}

	disk_write(fs->disk, (char *) che_buf, file->location + fil_sec,
		che_sec);
	free(che_buf); */
	return FS_OK;
}

enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file)
{
	uint8_t *tmp;

	tmp = malloc(fs->sector_size);
	memcpy(tmp, file, sizeof(struct INODE));

	disk_write(fs->disk, (char *) tmp, fs->inode_block +
	file->inode_offset, 1);
	free(tmp);
	file = NULL;
	return FS_OK;
}

enum FSRESULT fs_mount(struct disk *disk, struct FILE_SYSTEM *fs)
{

	unsigned long sector_size, super_size;
	char *buffer;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	super_size = div_up(sizeof(struct FILE_SYSTEM), sector_size);
	buffer = (char *) malloc(super_size * sector_size);

	disk_read(disk, buffer, 0, super_size);

	memcpy(fs, buffer, sizeof(struct FILE_SYSTEM));
	fs->disk = disk; /*TODO: Remove this in the working system */

	free(buffer);
	buffer = NULL;

	return FS_OK;
}

unsigned long fs_getfree(struct FILE_SYSTEM *fs)
{
	unsigned long tmp, byte_cnt;
	char *buffer;
	uint i;

	byte_cnt = div_up(fs->sector_count, 8);

	buffer = (char *) malloc(fs->alloc_table_size * fs->sector_size);
	disk_read(fs->disk, buffer, fs->alloc_table, fs->alloc_table_size);

	tmp = 0;
	for (i = 0; i < byte_cnt; ++i)
		tmp += (8 - popcount((uint8_t) buffer[i]));

	free(buffer);
	return tmp * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
unsigned long size, uint time_to_live, bool custody)
{
	int all_off, ino_off;
	uint bit_cnt, byt_ino_tab, byt_all_tab, check_size;
	uint8_t *ino_tab, *all_tab;
	enum FSRESULT ret_val;

	check_size = checksum_size(size);
	bit_cnt = div_up(size, fs->sector_size);
	//TODO: No longer needed
	//bit_cnt += div_up(check_size, fs->sector_size);
	all_tab  = malloc(fs->alloc_table_size * fs->sector_size);
	ino_tab  = malloc(fs->inode_alloc_table_size * fs->sector_size);

	disk_read(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);
	disk_read(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	/*Finds and writes a free inode in the inode alloc table*/
	byt_ino_tab  = fs->inode_alloc_table_size * fs->sector_size;
	ino_off = find_bit(ino_tab, byt_ino_tab);
	if (ino_off < 0) {
		if (resize_inode_block(fs)) {
			disk_read(fs->disk, (char *) ino_tab,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size);
			/* This mount is needed because the metadata changed */
			fs_mount(fs->disk, fs);
			++byt_ino_tab;
			ino_off = find_bit(ino_tab, byt_ino_tab);
		} else {
		ret_val = FS_FULL;
		goto END;
		}
	}

	/*Finds a sequence of bits that are empty in the allocation table*/
	byt_all_tab  = fs->alloc_table_size * fs->sector_size;
	all_off = find_seq(all_tab, byt_all_tab, bit_cnt);

	if (all_off < 0) {
		if (free_disk_space(fs, size))
			disk_read(fs->disk, (char *) all_tab, fs->alloc_table,
				fs->alloc_table_size);
			all_off = find_seq(all_tab, byt_all_tab,
				bit_cnt);
			disk_read(fs->disk, (char *) ino_tab,
				fs->inode_alloc_table,
				fs->inode_alloc_table_size);
			ino_off = find_bit(ino_tab, byt_ino_tab);

		if (all_off < 0) {
			ret_val = FS_FULL;
			goto END;
		}
	}

	/* Write the buffer */
	write_seq(all_tab, all_off, bit_cnt);
	write_bit(ino_tab, ino_off, true);

	file->size = size;
	file->location = fs->sector_count - all_off - bit_cnt;
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = ino_off;

	/* Write all to disk */
	disk_write(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);
	write_inode(fs, file);

	ret_val = FS_OK;
END:
	free(ino_tab);
	free(all_tab);
	return ret_val;
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
	uint8_t *all_tab, *tmp_sec, *ino_tab;
	uint tmp, bits, at_size;

	bits = div_up(8, fs->inode_sec);
	at_size = fs->sector_size * fs->alloc_table_size;

	all_tab = malloc(at_size);
	disk_read(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);

	ino_tab = malloc(fs->sector_size * fs->inode_alloc_table_size);
	disk_read(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	tmp = fs->sector_count - (fs->inode_block + fs->inode_block_size);
	tmp -= bits;

	if (!check_seq(all_tab, tmp, bits)) {
		free(all_tab);
		free(ino_tab);
		return false;
	}

	/* TODO: Recover the wasted inode space */
	write_seq(all_tab, tmp, bits);

	ino_tab[fs->inode_max / 8] = 0x00;

	disk_write(fs->disk, (char *) all_tab, fs->alloc_table,
		fs->alloc_table_size);
	free(all_tab);

	disk_write(fs->disk, (char *) ino_tab, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	free(ino_tab);

	tmp_sec = malloc(fs->sector_size);
	fs->inode_block_size += bits;
	fs->inode_max += 8;
	memcpy(tmp_sec, fs, sizeof(struct FILE_SYSTEM));
	disk_write(fs->disk, (char *) tmp_sec, 0, 1);
	free(tmp_sec);

	return true;
}
