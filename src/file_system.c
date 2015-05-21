#include "../h/file_system.h"


inline unsigned long round_up_div(unsigned long dividend,
	unsigned long divisor)
{
	return (dividend + divisor - 1) / divisor;
}

int get_first_free_bit(uint8_t byte)
{
	int i;
	uint8_t temp = 0xFF;

	for (i = 8; i >= 0; --i) {
		if ((byte & (temp << i)) != 0x00)
			return (i * (-1)) + 7;
	}
	return 8;
}

int get_last_free_bit(uint8_t byte)
{
	int i;
	uint8_t temp = 0xFF;

	for (i = 8; i >= 0; --i) {
		if ((byte & (temp >> i)) != 0x00)
			return (i * (-1)) + 7;
	}
	return 8;
}

/*stolen from the internet, counts the 1 bits in a byte*/
int popcount(unsigned char x)
{
	return ((0x876543210 >>
	(((0x4332322132212110 >> ((x & 0xF) << 2)) & 0xF) << 2)) >>
	((0x4332322132212110 >> (((x & 0xF0) >> 2)) & 0xF) << 2))
	& 0xf;
}

enum FSRESULT fs_mkfs(struct disk *disk)
{

	/*Check if the drive is ready*/
	unsigned int i;
	struct FILE_SYSTEM *fs;/*the file system to be created*/
	unsigned long sector_count, sector_size, max_file_count;
	unsigned long size, dif;
	unsigned long superblock_size; /*Size of the superblock in sectors*/
	char *temparray; /*Used to make the fs superblock*/

	fs = (*struct FILE_SYSTEM) malloc(sizeof(struct FILE_SYSTEM));
	/*Gets the sector size*/
	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	superblock_size = round_up_div(sizeof(struct FILE_SYSTEM), sector_size);

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK)
		return FS_ERROR;
	/*Calculate the fs size and make fs_info*/
	/*Make the allocation table*/
	/*TODO:Change this to a better thing*/
	max_file_count = sector_count / 8;
	if (max_file_count < 8)
		max_file_count = 8;
	/*The number of sectors needed for the allocation table*/
	fs->alloc_table_size = round_up_div(sector_count, sector_size * 8);
	fs->alloc_table = superblock_size; /*first comes the superblock*/
	size = sector_size * fs->alloc_table_size;
	/*Calculates the padding of the sector*/
	dif = round_up_div(sector_count, 8) % sector_size;
	dif = (fs->alloc_table_size - 1) * sector_size + dif;
	if (sector_count % 8 != 0)
		/*TODO:This should never happen if you used the right
		numbers for the fs */
		dif = dif - 1;
	temparray = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			temparray[i] = 0xFF;
		else
			temparray[i] = 0x00;
	}

	disk_write(disk, temparray, fs->alloc_table, fs->alloc_table_size);
	free(temparray);

	/*Make inode_alloc_table*/
	fs->inode_alloc_table_size = round_up_div(max_file_count,
		sector_size * 8);
	fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;

	size = sector_size * fs->inode_alloc_table_size;

	dif = round_up_div(max_file_count, 8) % sector_size;
	dif = (fs->inode_alloc_table_size - 1) * sector_size + dif;
	if (sector_count % 8 != 0) /*TODO: This should never happen. */
		dif = dif - 1;
	temparray = malloc(sizeof(char) * size);
	for (i = 0; i < size; ++i) {
		if (i >= dif)
			temparray[i] = 0xFF;
		else
			temparray[i] = 0x00;
	}

	disk_write(disk, temparray, fs->inode_alloc_table,
	fs->inode_alloc_table_size);
	free(temparray);

	/*Make Inode Block*/
	fs->inode_block_size = max_file_count;
	fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
	fs->sector_size = sector_size;
	size = sector_size * fs->inode_block_size;
	temparray = malloc(sizeof(struct INODE) * size);
	for (i = 0; i < size; ++i)
		temparray[i] = 0x0;

	disk_write(disk, temparray, fs->inode_block, fs->inode_block_size);
	free(temparray);
	/*Make superblock*/
	disk_write(disk, (*char) fs, 0, superblock_size);
	/*TODO: Make a padding, so no random memory is writen to disk*/
	free(fs);
	fs = NULL;
	return FS_OK;

}

enum FSRESULT fs_open(struct FILE_SYSTEM *fs, unsigned long number,
	struct INODE *file)
{
	unsigned int i;
	uint8_t *buffer;
	struct INODE *temp;

	buffer = malloc(fs->sector_size * fs->inode_block_size);
	disk_read(fs->disk, (*char) buffer, fs->inode_block,
		fs->inode_block_size);

	for (i = 0; i < fs->inode_block_size; ++i) {
		temp = (*struct INODE) buffer;
		/*buffer += fs->sector_size*/
		if (temp->id == number)
			break;
	}
	/*Resets the pointer, so that free can be called*/
	buffer -= fs->sector_size * i;

	if (i >= fs->inode_block_size)
		return FS_ERROR;

	file->id = temp->id;
	file->size = temp->size;
	file->creation_date = temp->creation_date;
	file->last_modified = temp->last_modified;
	file->offset = 0;
	file->location = temp->location;
	file->custody = temp->custody;
	file->time_to_live = temp->time_to_live;

	free(buffer);
	buffer = NULL;
	return FS_OK;
}

enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file)
{
	/*Free bits in alloc_table*/
	unsigned int bit_count, byte_count, temp_bit_count,
	startpadding, i, k, temp_offset;
	uint8_t *alloc_table;
	uint8_t *inode_alloc_table;
	uint8_t *buffer;
	uint8_t temp_byte;

	alloc_table = malloc(fs->sector_size * fs->alloc_table_size);
	disk_read(fs->disk, (*char) alloc_table, fs->alloc_table,
		fs->alloc_table_size);

	bit_count = round_up_div(file->size, fs->sector_size);
	byte_count = round_up_div(bit_count, 8);

	temp_bit_count = bit_count;
	startpadding = 8 - (file->location % 8);
	k = round_up_div(file->location, 8);
	/*Writes the bits into the alloc_table*/
	for (i = k; i < (k + byte_count); ++i) {
		/*Start*/
		if (temp_bit_count == bit_count) {
			if (temp_bit_count > startpadding) {
				temp_byte = 0xFF >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] ^ temp_byte;
				temp_bit_count -= startpadding;
			} else {
				temp_byte = 0xFF << (8 - temp_bit_count);
				temp_byte = temp_byte >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] ^ temp_byte;
			}
		} else {
			/*End*/
			if (i == (byte_count - 1)) {
				temp_byte = 0xFF << (8 - temp_bit_count);
				alloc_table[i] = alloc_table[i] ^ temp_byte;
			} /*Middle*/else {
				alloc_table[i] = 0xFF;
				temp_bit_count -= 8;
			}
		}
	}

	disk_write(fs->disk, (*char) alloc_table, fs->alloc_table,
		fs->alloc_table_size);
	free(alloc_table);

	/*Free bit in inode_table*/
	inode_alloc_table = malloc(fs->sector_size *
		fs->inode_alloc_table_size);
	disk_read(fs->disk, (*char) inode_alloc_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	temp_byte = 0x80 >> (file->inode_offset % 8);
	temp_offset = file->inode_offset / 8;
	inode_alloc_table[temp_offset] = inode_alloc_table[temp_offset] ^
	temp_byte;

	disk_write(fs->disk, (*char) inode_alloc_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	free(inode_alloc_table);

	/* Delete Inode */
	buffer = malloc(fs->sector_size);
	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x00;

	disk_write(fs->disk, (*char) buffer, fs->inode_block +
		file->inode_offset, 1);

	free(buffer);
	file->location = 0xFFFFFFFF;

	return FS_ERROR;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	unsigned long sector_count;
	unsigned int offset;

	offset = fs->inode_block + fs->inode_block_size;
	sector_count = round_up_div(file->size, fs->sector_size);
	/*TODO: Breaks if the buffer has the wrong size */
	disk_read(fs->disk, buffer, file->location + offset, sector_count);
	return FS_OK;
}

enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	unsigned long sector_count;
	unsigned int offset;

	offset = fs->inode_block + fs->inode_block_size;
	sector_count = round_up_div(file->size, fs->sector_size);
	file->last_modified = (unsigned int) time(NULL);

	disk_write(fs->disk, buffer, offset + file->location, sector_count);
	return FS_OK;
}

enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file)
{
	disk_write(fs->disk, (*char) file, fs->inode_block + file->inode_offset,
	 1);
	return FS_OK;
}

enum FSRESULT fs_mount(struct disk *disk, struct FILE_SYSTEM *fs)
{

	unsigned long sector_size, superblock_size;
	char *buffer;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	superblock_size = round_up_div(sizeof(struct FILE_SYSTEM), sector_size);
	buffer = (*char) malloc(superblock_size * sector_size);

	disk_read(disk, buffer, 0, superblock_size);
	/*This is ugly but memcpy makes fuuuu */
	struct FILE_SYSTEM *tempFS = (*struct FILE_SYSTEM) buffer;

	fs->alloc_table = tempFS->alloc_table;
	fs->alloc_table_size = tempFS->alloc_table_size;
	fs->inode_alloc_table = tempFS->inode_alloc_table;
	fs->inode_alloc_table_size = tempFS->inode_alloc_table_size;
	fs->inode_block = tempFS->inode_block;
	fs->inode_block_size = tempFS->inode_block_size;
	fs->sector_size = tempFS->sector_size;

	free(buffer);
	buffer = NULL;

	return FS_OK;
}

unsigned long fs_getfree(struct disk *disk, struct FILE_SYSTEM *fs)
{
	unsigned long sector_size, ret, sector_count, byte_count;
	char *buffer;
	unsigned int i;

	if (disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK)
		return FS_ERROR;

	byte_count = round_up_div(sector_count, 8);

	buffer = (*char) malloc(fs->alloc_table_size * sector_size);
	disk_read(disk, buffer, fs->alloc_table, fs->alloc_table_size);

	ret = 0;
	for (i = 0; i < byte_count; ++i)
		ret += (8 - popcount(buffer[i]));

	free(buffer);
	return ret * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *inode,
unsigned long size, unsigned int time_to_live, short custody)
{
	unsigned long sector_size;
	unsigned int allocation_offset, inode_offset, temp_bit_count,
	startpadding;
	unsigned int i, k, bit_count, byte_count, bytes_inode_alloc_table;
	uint8_t temp_byte;
	uint8_t *inode_buf;
	uint8_t *inode_alloc_table;
	uint8_t *alloc_table;
	uint8_t *buffer;

	if (disk_ioctl(fs->disk, GET_SECTOR_SIZE, &sector_size) != RES_OK)
		return FS_ERROR;

	alloc_table = malloc(fs->alloc_table_size * sector_size);
	disk_read(fs->disk, (*char) alloc_table, fs->alloc_table,
		fs->alloc_table_size);

	bit_count = round_up_div(size, sector_size);
	byte_count = round_up_div(bit_count, 8);

	inode_alloc_table = malloc(fs->inode_alloc_table_size * sector_size);

	disk_read(fs->disk, (*char) inode_alloc_table, fs->inode_alloc_table,
	 fs->inode_alloc_table_size);

	bytes_inode_alloc_table = (fs->inode_alloc_table_size * sector_size);

	inode_offset = 0;
	for (i = 0; i < bytes_inode_alloc_table; ++i) {
		if (inode_alloc_table[i] != 0xFF)
			break;
	}

	/*No free inodes*/
	if ((i >= bytes_inode_alloc_table) && (inode_alloc_table[i-1] == 0xFF))
		return FS_ERROR;

	temp_byte = 0;
	for (k = 0; k < 8; ++k) {
		temp_byte = 0x80 >> k;
		if ((inode_alloc_table[i] & temp_byte) == 0x00) {
			inode_alloc_table[i] = inode_alloc_table[i] | temp_byte;
			inode_offset = (8 * i) + k;
			break;
		}
	}
	disk_write(fs->disk, (*char) inode_alloc_table, fs->inode_alloc_table,
	fs->inode_alloc_table_size);
	free(inode_alloc_table);

	/*Finds a sequence of bits that are empty in the allocation table*/
	k = 0;
	temp_bit_count = 0;
	while (k < (fs->alloc_table_size * sector_size)) {
		if ((alloc_table[k] & 0xFF) == 0x00) {
			temp_bit_count += 8;
		} else {
			/*Start of byte*/
			if (temp_bit_count == 0) {
				temp_bit_count += get_last_free_bit(
					alloc_table[k]);
			} else {
				/*End of byte*/
				temp_bit_count += get_first_free_bit(
					alloc_table[k]);
				if (temp_bit_count >= bit_count)
					break;
				temp_bit_count = 0;
			}
		}
		/*End normal*/
		if (temp_bit_count >= bit_count)
			break;
		k += 1;
	}

	if (temp_bit_count < bit_count) {
		free(alloc_table);
		/*TODO: Das bit in der inode table wieder freigeben*/
		return FS_ERROR;
	}

	k = (k - byte_count) + 1;

	temp_bit_count = bit_count;
	startpadding = get_last_free_bit(alloc_table[k]);
	allocation_offset = (8 * k) + (8 - startpadding);
	/*Writes the bits into the alloc_table*/
	for (i = k; i < (k + byte_count); ++i) {
		/*Start*/
		if (temp_bit_count == bit_count) {
			if (temp_bit_count > startpadding) {
				temp_byte = 0xFF >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] | temp_byte;
				temp_bit_count -= startpadding;
			} else {
				temp_byte = 0xFF << (8 - temp_bit_count);
				temp_byte = temp_byte >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] | temp_byte;
			}
		} else {
			/*End*/
			if (i == (byte_count - 1)) {
				temp_byte = 0xFF << (8 - temp_bit_count);
				alloc_table[i] = alloc_table[i] | temp_byte;
			} /*Middle*/else {
				alloc_table[i] = 0xFF;
				temp_bit_count -= 8;
			}
		}
	}

	disk_write(fs->disk, (*char) alloc_table, fs->alloc_table,
		fs->alloc_table_size);
	free(alloc_table);

	inode->size = size;
	inode->creation_date = (unsigned int) time(NULL);
	inode->last_modified = (unsigned int) time(NULL);
	inode->offset = 0;
	inode->location = allocation_offset;
	inode->custody = 0;
	inode->time_to_live = time_to_live;
	inode->inode_offset = inode_offset;

	/*Write to disk*/
	inode_buf = (*uint8_t) inode;
	buffer = malloc(sector_size);

	for (i = 0; i < sizeof(struct INODE); ++i)
		buffer[i] = inode_buf[i];

	disk_write(fs->disk, (*char) buffer, fs->inode_block + inode_offset, 1);
	free(buffer);

	return FS_OK;
}
/*
enum FSRESULT fs_seek(struct INODE* file,int offset, enum SEEK_MODE mod)
{
	unsigned int temp;
	switch(mod){
		case SEEK_CUR:
			temp = file->offset + offset;
			if(temp < file->size){
				file->offset = temp;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		case SEEK_SET:
			if(offset < file->size && offset >= 0){
				file->offset=offset;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		case SEEK_END:
			temp = file->offset + offset;
			if(temp < file->size && file->offset <= offset){
				file->offset = temp;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		default:
			return FS_PARAM_ERROR;
	}
}

//TODO: Wie verhindere ich das irgendwer einfach in der Inode rumschreibt?
unsigned long fs_tell(struct INODE* file)
{
	return (long) file->offset;
}*/
