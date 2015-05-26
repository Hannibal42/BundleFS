#include "../h/file_system.h"


/*Writes the given bit into the allocation table*/
void write_bit(uint index, uint8_t *table, bool value);
/* Finds the first free bit */
int find_bit(uint8_t *table, uint length);
/* Finds the first free sequence of the given length */
int find_sequence(uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
void write_seq(uint index, uint length, uint8_t *table);
/* Lets the bits toggle */
void delete_seq(uint index, uint length, uint8_t *table);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
int find_sequence_small(uint8_t *table, uint table_size, uint length);
inline unsigned long div_up(unsigned long dividend,
	unsigned long divisor);

enum FSRESULT fs_mkfs(struct disk *disk)
{
	/*Check if the drive is ready*/
	unsigned int i;
	struct FILE_SYSTEM *fs;/*the file system to be created*/
	unsigned long sector_count, sector_size, max_file_count;
	unsigned long size, dif;
	unsigned long superblock_size; /*Size of the superblock in sectors*/
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
	unsigned int i;
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


	if (i >= fs->inode_block_size)
		return FS_ERROR;
	/* TODO: Copy the memory direct */
	file->id = tmp->id;
	file->size = tmp->size;
	file->creation_date = tmp->creation_date;
	file->inode_offset = tmp->inode_offset;
	file->last_modified = tmp->last_modified;
	file->offset = 0;
	file->location = tmp->location;
	file->custody = tmp->custody;
	file->time_to_live = tmp->time_to_live;

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
	delete_seq(file->location, bit_length, alloc_table);

	/*Free bit in inode_table*/
	IN_table = malloc(fs->sector_size * fs->inode_alloc_table_size);
	disk_read(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	write_bit(file->inode_offset, IN_table, false);

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

	/* TODO: Define a good error value */
	file->location = 0xFFFFFFFF;

	return FS_OK;
}

enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer)
{
	unsigned long sector_count;
	unsigned int offset;

	offset = fs->inode_block + fs->inode_block_size;
	sector_count = div_up(file->size, fs->sector_size);
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
	sector_count = div_up(file->size, fs->sector_size);
	file->last_modified = (uint) time(NULL);

	disk_write(fs->disk, buffer, offset + file->location, sector_count);
	return FS_OK;
}

enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file)
{
	disk_write(fs->disk, (char *) file, fs->inode_block +
	file->inode_offset, 1);
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

	struct FILE_SYSTEM *tmpFS = (struct FILE_SYSTEM *) buffer;

	fs->alloc_table = tmpFS->alloc_table;
	fs->alloc_table_size = tmpFS->alloc_table_size;
	fs->inode_alloc_table = tmpFS->inode_alloc_table;
	fs->inode_alloc_table_size = tmpFS->inode_alloc_table_size;
	fs->inode_block = tmpFS->inode_block;
	fs->inode_block_size = tmpFS->inode_block_size;
	fs->sector_size = tmpFS->sector_size;

	free(buffer);
	buffer = NULL;

	return FS_OK;
}

unsigned long fs_getfree(struct disk *disk, struct FILE_SYSTEM *fs)
{
	unsigned long ret, sector_count, byte_count;
	char *buffer;
	unsigned int i;

	/*Get number of sectors*/
	if (disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK)
		return FS_ERROR;

	byte_count = div_up(sector_count, 8);

	buffer = (char *) malloc(fs->alloc_table_size * fs->sector_size);
	disk_read(disk, buffer, fs->alloc_table, fs->alloc_table_size);

	ret = 0;
	for (i = 0; i < byte_count; ++i)
		ret += (8 - popcount((uint8_t) buffer[i]));

	free(buffer);
	return ret * fs->sector_size;
}

enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
unsigned long size, uint time_to_live, bool custody)
{
	uint i, AL_off, IN_off, bit_count, bytes_IN_table,
	bytes_AL_table;
	uint8_t *IN_buf, *IN_table, *AL_table, *buffer;

	bit_count = div_up(size, fs->sector_size);
	AL_table  = malloc(fs->alloc_table_size * fs->sector_size);
	IN_table  = malloc(fs->inode_alloc_table_size * fs->sector_size);

	disk_read(fs->disk, (char *) AL_table, fs->alloc_table,
		fs->alloc_table_size);
	disk_read(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	/*Finds and writes a free inode in the inode alloc table*/
	bytes_IN_table  = fs->inode_alloc_table_size * fs->sector_size;
	IN_off = find_bit(IN_table, bytes_IN_table);
	if (IN_off < 0) {
		free(AL_table);
		free(IN_table);
		return FS_ERROR;
	}
	write_bit(IN_off, IN_table, true);

	/*Finds a sequence of bits that are empty in the allocation table*/
	bytes_AL_table  = fs->alloc_table_size * fs->sector_size;
	AL_off = find_sequence(AL_table, bytes_AL_table, bit_count);
	if (AL_off < 0) {
		free(AL_table);
		free(IN_table);
		return FS_ERROR;
	}
	write_seq(AL_off, bit_count, AL_table);

	file->size = size;
	file->creation_date = (uint) time(NULL);
	file->last_modified = (uint) time(NULL);
	file->offset = 0;
	file->location = AL_off;
	file->custody = custody;
	file->time_to_live = time_to_live;
	file->inode_offset = IN_off;

	/*Write to disk*/
	IN_buf = (uint8_t *) file;
	buffer = malloc(fs->sector_size);

	for (i = 0; i < sizeof(struct INODE); ++i)
		buffer[i] = IN_buf[i];
	IN_buf = NULL;

	/* Write all to disk */
	disk_write(fs->disk, (char *) IN_table, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	disk_write(fs->disk, (char *) AL_table, fs->alloc_table,
		fs->alloc_table_size);
	disk_write(fs->disk, (char *) buffer, fs->inode_block + IN_off, 1);
	free(IN_table);
	free(AL_table);
	free(buffer);

	return FS_OK;
}


/*Writes the given bit into the allocation table*/
void write_bit(uint index, uint8_t *table, bool bit_value)
{
	uint8_t tmp_byte;
	uint byte_index;
	uint bit_index;

	byte_index = index / 8;
	bit_index = index % 8;
	tmp_byte = 0x80 >> bit_index;

	if (bit_value) {
		table[byte_index] |= tmp_byte;
	} else {
		tmp_byte ^= 0xFF;
		table[byte_index] &= tmp_byte;
	}
}

/*Finds the next free bit in the allocation table*/
int find_bit(uint8_t *table, uint size)
{
	uint i, k;
	uint8_t tmp_byte;

	for (i = 0; i < size; ++i) {
		if (table[i] != 0xFF)
			break;
	}

	/*No free inodes*/
	if (i >= size)
		return -1;

	for (k = 0; k < 8; ++k) {
		tmp_byte = 0x80 >> k;
		if ((table[i] & tmp_byte) == 0x00)
			return (8 * i) + k;
	}
	return -1;/* This should never happen */
}

int find_sequence_byte(uint8_t byte, uint length)
{
	uint i, tmp;
	uint8_t tmp_byte;

	tmp = 0;
	for (i = 0; i < 8; ++i) {
		tmp_byte = 0x80 >> i;
		if ((byte & tmp_byte) == 0x00)
			tmp += 1;
		else
			tmp = 0;

		if (tmp >= length)
			return i - (tmp-1);
	}
	return -1;
}

/* lenght must be < 9 for this, and the table must be > 0*/
int find_sequence_small(uint8_t *table, uint table_size, uint length)
{
	uint i;
	int tmp;

	tmp = find_sequence_byte(table[0], length);
	if (tmp > 0)
		return tmp;
	tmp = last_free_bits(table[0]);
	for (i = 1; i < table_size; ++i) {
		/* Small optimization */
		if (table[i] == 0xFF) {
			tmp = 0;
			continue;
		}

		tmp += first_free_bits(table[i]);
		if (tmp >= length)
			return i * 8 - last_free_bits(table[i-1]);
		tmp = find_sequence_byte(table[i], length);
		if (tmp > 0)
			return i * 8 + tmp;
		tmp = last_free_bits(table[i]);
	}
	return -1;
}

/* TODO: Rewrite this shameful function */
int find_sequence(uint8_t *table, uint table_size, uint length)
{
	int tmp;
	uint i, start;

	if (length < 9)
		return find_sequence_small(table, table_size, length);

	tmp = 0;
	start = 0;
	for (i = 0; i < table_size; ++i) {
		if (table[i] == 0x00) {
			tmp += 8;
			if (tmp >= length)
				return (start * 8) +
				(8 - last_free_bits(table[start]));
		} else {
			tmp += first_free_bits(table[i]);
			if (tmp >= length)
				return (start * 8) +
				(8 - last_free_bits(table[start]));
			tmp = last_free_bits(table[i]);
			start = i;
		}
	}
	return -1;
}

int get_free_bit(uint8_t index, uint8_t byte)
{
	int i;
	uint8_t tmp = 0x80;

	if (index > 7)
		return -1;

	for (i = index; i < 8; ++i) {
		if ((byte & (tmp >> i)) != 0x00)
			return (i - index);
	}
	return (i - index);
}

void write_seq(uint index, uint length, uint8_t *table)
{
	uint tmp, startpadding, i, start_byte, end_byte;
	uint8_t tmp_byte;

	start_byte = index / 8;
	tmp = length;
	startpadding = 8 - (index % 8);
	end_byte = start_byte + ((length + (index % 8)) / 8);

	/* Start byte */
	if (tmp > startpadding) {
		tmp_byte = 0xFF >> (8 - startpadding);
		tmp -= startpadding;
	} else {
		tmp_byte = 0xFF << (8 - tmp);
		tmp_byte = tmp_byte >> (8 - startpadding);
		tmp = 0;
	}
	table[start_byte] |= tmp_byte;

	for (i = (start_byte + 1); i <= end_byte; ++i) {
		if (i == end_byte) {
			tmp_byte = 0xFF << (8 - tmp);
			table[i] |= tmp_byte;
		} else {
			table[i] = 0xFF;
			tmp -= 8;
		}
	}
}

void delete_seq(uint index, uint length, uint8_t *table)
{
	uint tmp, startpadding, i, start_byte, end_byte;
	uint8_t tmp_byte;

	start_byte = index / 8;
	tmp = length;
	startpadding = 8 - (index % 8);
	end_byte = start_byte + ((length + (index % 8)) / 8);

	/* Start byte */
	if (tmp > startpadding) {
		tmp_byte = 0xFF << startpadding;
		tmp -= startpadding;
	} else {
		tmp_byte = 0xFF << startpadding;
		tmp = 8 - (startpadding - tmp);
		tmp_byte |= 0xFF >> tmp;
		tmp = 0;
	}
	table[start_byte] &= tmp_byte;

	for (i = (start_byte + 1); i <= end_byte; ++i) {
		if (i == end_byte) {
			tmp_byte = 0xFF >> tmp;
			table[i] &= tmp_byte;
		} else {
			table[i] = 0x00;
			tmp -= 8;
		}
	}
}

inline unsigned long div_up(unsigned long dividend,
	unsigned long divisor)
{
	return (dividend + divisor - 1) / divisor;
}

int first_free_bits(uint8_t byte)
{
	return get_free_bit(0, byte);
}

int last_free_bits(uint8_t byte)
{
	int i;
	uint8_t tmp = 0xFF;

	for (i = 8; i >= 0; --i) {
		if ((byte & (tmp >> i)) != 0x00)
			return 7 - i;
	}
	return 8;
}

/*stolen from the internet, counts the 1 bits in a byte*/
int popcount(uint8_t byte)
{
	return ((0x876543210 >>
	(((0x4332322132212110 >> ((byte & 0xF) << 2)) & 0xF) << 2)) >>
	((0x4332322132212110 >> (((byte & 0xF0) >> 2)) & 0xF) << 2))
	& 0xf;
}

/*
enum FSRESULT fs_seek(struct INODE* file,int offset, enum SEEK_MODE mod)
{
	unsigned int tmp;
	switch(mod){
		case SEEK_CUR:
			tmp = file->offset + offset;
			if(tmp < file->size){
				file->offset = tmp;
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
			tmp = file->offset + offset;
			if(tmp < file->size && file->offset <= offset){
				file->offset = tmp;
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
