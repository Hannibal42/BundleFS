#include "file_system.h"

bool free_disk_space(struct disk *disk, struct FILE_SYSTEM *fs, uint size);
void delete_invalid_inodes(struct disk *disk, struct FILE_SYSTEM *fs);
struct INODE *load_inodes_all(struct FILE_SYSTEM *fs);
bool isNotValid(struct INODE *inode);
bool find_first_IN_length(struct disk *disk, struct FILE_SYSTEM *fs,
	struct INODE *file, uint size);

enum FSRESULT fs_mkfs(struct disk *disk)
{
	/*Check if the drive is ready*/
	uint i;
	struct FILE_SYSTEM *fs;/*the file system to be created*/
	unsigned long sector_count, sector_size, max_file_count, size,
	dif, superblock_size;/*Size of the superblock in sectors*/
	char *tmparray; /*Used to make the fs superblock*/

	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK)
		return FS_ERROR;

	fs = (struct FILE_SYSTEM *) malloc(sizeof(struct FILE_SYSTEM));
	superblock_size = div_up(sizeof(struct FILE_SYSTEM), sector_size);

	/*TODO:Change this to a better thing*/
	max_file_count = sector_count / 8;
	if (max_file_count < 8)
		max_file_count = 8;

	/*The number of sectors needed for the allocation table*/
	/* TODO: Substract the superblock, and inode_block size*/
	fs->alloc_table_size = div_up(sector_count, sector_size * 8);
	fs->alloc_table = superblock_size; /*first comes the superblock*/
	size = sector_size * fs->alloc_table_size;
	/*Calculates the padding of the sector*/
	dif = div_up(sector_count, 8) % sector_size;
	dif = (fs->alloc_table_size - 1) * sector_size + dif;
	if (sector_count % 8 != 0)
		/*TODO:This should never happen if you used the right
		numbers for the fs */
		dif = dif - 1;
	tmparray = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			tmparray[i] = 0xFF;
		else
			tmparray[i] = 0x00;
	}

	disk_write(disk, tmparray, fs->alloc_table, fs->alloc_table_size);
	free(tmparray);

	/*Make inode_alloc_table*/
	fs->inode_alloc_table_size = div_up(max_file_count,
		sector_size * 8);
	fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;

	size = sector_size * fs->inode_alloc_table_size;

	dif = div_up(max_file_count, 8) % sector_size;
	dif = (fs->inode_alloc_table_size - 1) * sector_size + dif;
	if (sector_count % 8 != 0) /*TODO: This should never happen. */
		dif = dif - 1;
	tmparray = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			tmparray[i] = 0xFF;
		else
			tmparray[i] = 0x00;
	}
	disk_write(disk, tmparray, fs->inode_alloc_table,
	fs->inode_alloc_table_size);
	free(tmparray);

	/*Make Inode Block*/
	fs->inode_block_size = max_file_count;
	fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
	fs->sector_size = sector_size;
	fs->sector_count = sector_count;
	size = sector_size * fs->inode_block_size;
	tmparray = malloc(sizeof(struct INODE) * size);
	for (i = 0; i < size; ++i)
		tmparray[i] = 0x0;

	disk_write(disk, tmparray, fs->inode_block, fs->inode_block_size);
	free(tmparray);
	/*Make superblock*/
	disk_write(disk, (char *) fs, 0, superblock_size);
	/*TODO: Make a padding, so no random memory is writen to disk*/
	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number,
	struct INODE *file)
{
	uint i;
	uint8_t *buffer;
	struct INODE *tmp;

	buffer = malloc(fs->sector_size * fs->inode_block_size);
	disk_read(fs->disk, (char *) buffer, fs->inode_block,
		fs->inode_block_size);

	for (i = 0; i < fs->inode_block_size; ++i) {
		tmp = (struct INODE *) buffer;
		buffer += fs->sector_size;
		if (tmp->id == number)
			break;
	}
	/*Resets the pointer, so that free can be called*/
	buffer -= fs->sector_size * (i + 1);

	if (i >= fs->inode_block_size) {
		buffer += fs->sector_size;
		free(buffer);
		return FS_ERROR;
	}

	memcpy(file, tmp, sizeof(struct INODE));

	free(buffer);
	buffer = NULL;
	return FS_OK;
}

enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file)
{
	/*Free bits in alloc_table*/
	uint i, bit_length;
	uint8_t *alloc_table, *IN_table, *buffer;

	/* Free bits in the allocation table */
	alloc_table = malloc(fs->sector_size * fs->alloc_table_size);
	disk_read(fs->disk, (char *) alloc_table, fs->alloc_table,
		fs->alloc_table_size);
	bit_length = div_up(file->size, fs->sector_size);
	bit_length += div_up(file->check_size, fs->sector_size);
	delete_seq(alloc_table, file->location, bit_length);

	/*Free bit in inode_table*/
	IN_table = malloc(fs->sector_size * fs->inode_alloc_table_size);
	disk_read(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	write_bit(IN_table, file->inode_offset, false);

	/* Delete Inode */
	buffer = malloc(fs->sector_size);
	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x00;

	/* TODO: Is this a correct softupdate? */
	disk_write(fs->disk, (char *) alloc_table, fs->alloc_table,
		fs->alloc_table_size);
	disk_write(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) buffer, fs->inode_block +
		file->inode_offset, 1);
	free(alloc_table);
	free(IN_table);
	free(buffer);

	/* TODO: Define a good error value, or free inode*/
	buffer = (uint8_t *) file;
	for (i = 0; i < sizeof(struct INODE); ++i)
		buffer[i] = 0xFF;

	return FS_OK;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer, uint length)
{
	uint i, sector_count;
	uint8_t *tmp;

	if (length > file->size)
		return FS_PARAM_ERROR;

	sector_count = div_up(file->size, fs->sector_size);
	sector_count += div_up(file->check_size, fs->sector_size);
	tmp = malloc(sector_count * fs->sector_size);

	disk_read(fs->disk, (char *) tmp, file->location, sector_count);
	if (!checksum_check(tmp, file, fs->sector_size)) {
		free(tmp);
		return FS_CHECK_ERROR;
	}

	for (i = 0; i < length; ++i)
		buffer[i] = tmp[i];

	free(tmp);
	return FS_OK;
}

enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	uint sector_count_file, sector_count_check;
	uint8_t *tmp;

	sector_count_file = div_up(file->size, fs->sector_size);
	sector_count_check = div_up(file->check_size, fs->sector_size);

	tmp = malloc(sector_count_check * fs->sector_size);
	checksum((uint8_t *) buffer, file->size, tmp, file->check_size);
	file->last_modified = (uint) time(NULL);

	disk_write(fs->disk, buffer, file->location, sector_count_file);
	disk_write(fs->disk, (char *) tmp, file->location +
		sector_count_file, sector_count_check);
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

	unsigned long sector_size, superblock_size;
	char *buffer;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	superblock_size = div_up(sizeof(struct FILE_SYSTEM), sector_size);
	buffer = (char *) malloc(superblock_size * sector_size);

	disk_read(disk, buffer, 0, superblock_size);

	memcpy(fs, buffer, sizeof(struct FILE_SYSTEM));
	fs->disk = disk; /*TODO: Remove this in the working system */

	free(buffer);
	buffer = NULL;

	return FS_OK;
}

unsigned long fs_getfree(struct FILE_SYSTEM *fs)
{
	unsigned long tmp, byte_count;
	char *buffer;
	uint i;

	byte_count = div_up(fs->sector_count, 8);

	buffer = (char *) malloc(fs->alloc_table_size * fs->sector_size);
	disk_read(fs->disk, buffer, fs->alloc_table, fs->alloc_table_size);

	tmp = 0;
	for (i = 0; i < byte_count; ++i)
		tmp += (8 - popcount((uint8_t) buffer[i]));

	free(buffer);
	return tmp * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
unsigned long size, uint time_to_live, bool custody)
{
	uint AL_off, IN_off, bit_count, bytes_IN_table, bytes_AL_table,
	check_size;
	uint8_t *IN_table, *AL_table, *buffer;
	enum FSRESULT ret_val;

	/* TODO: Use the right calculation */
	check_size = size / 8;
	bit_count = div_up(size, fs->sector_size);
	bit_count += div_up(check_size, fs->sector_size);
	AL_table  = malloc(fs->alloc_table_size * fs->sector_size);
	IN_table  = malloc(fs->inode_alloc_table_size * fs->sector_size);

	disk_read(fs->disk, (char *) AL_table, fs->alloc_table,
		fs->alloc_table_size);
	disk_read(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	/*Finds and writes a free inode in the inode alloc table*/
	bytes_IN_table  = fs->inode_alloc_table_size * fs->sector_size;
	IN_off = find_bit(IN_table, bytes_IN_table);
	/*Finds a sequence of bits that are empty in the allocation table*/
	bytes_AL_table  = fs->alloc_table_size * fs->sector_size;
	AL_off = find_sequence(AL_table, bytes_AL_table, bit_count);
	if (IN_off < 0) {
		/* Call the function to extend the inode block here */
		ret_val = FS_FULL;
		goto END;
	}
	/* TODO: Check if this is the right execution order...
		maybe I only want to delete if the new bundel has custody set*/
	if (AL_off < 0) {
		if (free_disk_space(fs->disk, fs, size))
			AL_off = find_sequence(AL_table, bytes_AL_table,
				bit_count);

		if (AL_off < 0) {
			ret_val = FS_FULL;
			goto END;
		}
	}

	/* Write the buffer */
	write_seq(AL_table, AL_off, bit_count);
	write_bit(IN_table, IN_off, true);

	file->size = size;
	file->check_size = check_size;
	file->creation_date = (uint) time(NULL);
	file->last_modified = (uint) time(NULL);
	file->location = fs->sector_count - AL_off -
		div_up(size, fs->sector_size);
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = IN_off;

	buffer = malloc(fs->sector_size);
	memcpy(buffer, file, sizeof(struct INODE));

	/* Write all to disk */
	disk_write(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) AL_table, fs->alloc_table,
		fs->alloc_table_size);
	disk_write(fs->disk, (char *) buffer, fs->inode_block + IN_off, 1);


	free(buffer);
	ret_val = FS_OK;

END:
	free(IN_table);
	free(AL_table);
	return ret_val;
}

bool free_disk_space(struct disk *disk, struct FILE_SYSTEM *fs, uint size)
{
	struct INODE *inode;

	inode = malloc(sizeof(struct INODE));

	if (!find_first_IN_length(disk, fs, inode, size))
		return false;

	/* TODO: Maybe write a better delete for this case */
	fs_delete(fs, inode);

	/* TODO: Notify upcn that a Budnle was deleted */

	free(inode);
	return true;
}

/* This should be a regular task executed by freeRTOS */
void delete_invalid_inodes(struct disk *disk, struct FILE_SYSTEM *fs)
{
	uint i, k;
	struct INODE *inodes;
	uint *tmp;

	tmp = malloc(2 * fs->inode_block_size);
	inodes = load_inodes_all(fs);

	k = 0;
	for (i = 0; i < fs->inode_block_size; ++i) {
		if (inodes[i].size > 0 && isNotValid(&inodes[i])) {
			tmp[k] = inodes[i].id;
			tmp[k + 1] = inodes[i].inode_offset;
			k += 2;
			/* TODO: Maybe write a better delete for this case */
			fs_delete(fs, &inodes[i]);
		}
	}

	/* TODO: Notify upcn which Bundles have expiered and where deleted. */
	free(tmp);
	free(inodes);
}

/* This needs a big buffer, TODO: Write more memory friednly
 TODO: Use an input buffer*/
struct INODE *load_inodes_all(struct FILE_SYSTEM *fs)
{
	uint i, k;
	uint8_t *tmp;
	struct INODE *ret_val;

	tmp = malloc(fs->inode_block_size * fs->sector_size);
	ret_val = malloc(fs->inode_block_size * sizeof(struct INODE));

	disk_read(fs->disk, (char *) tmp, fs->inode_block,
		fs->inode_block_size);

	k = 0;
	for (i = 0; i < fs->inode_block_size; ++i) {
		memcpy(&ret_val[i], &tmp[k], sizeof(struct INODE));
		k += fs->sector_size;
	}

	free(tmp);
	return ret_val;
}

/* Finds the first inode that can be deleted and returns that inode*/
bool find_first_IN_length(struct disk *disk,
	struct FILE_SYSTEM *fs, struct INODE *file, uint size)
{
	uint i;
	struct INODE *inodes;

	inodes = load_inodes_all(fs);

	for (i = 0; i < fs->inode_block; ++i) {
		if (inodes[i].size > size) {
			if (!inodes[i].custody || isNotValid(&inodes[i])) {
				memcpy(file, &inodes[i], sizeof(struct INODE));
				free(inodes);
				return true;
			}
		}
	}
	free(inodes);
	return false;
}

bool isNotValid(struct INODE *inode)
{
	uint t;
	/* TODO: Is the TTL a date or just how long the bundle lives? */
	t = (uint) time(NULL);

	return t > inode->time_to_live;
}
