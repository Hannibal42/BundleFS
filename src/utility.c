#include "utility.h"

#include <buffer.h>


/*Writes the given bit into the allocation table*/
void write_bit(uint8_t *table, uint index, bool bit_value)
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
int find_bit(const uint8_t *table, uint size)
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
	return -2;/* This should never happen */
}

int find_seq_byte(uint8_t byte, uint length)
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
int find_seq_small(const uint8_t *table, uint table_size, uint length)
{
	uint i;
	int tmp;

	tmp = find_seq_byte(table[0], length);
	if (tmp >= 0)
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
		tmp = find_seq_byte(table[i], length);
		if (tmp > 0)
			return i * 8 + tmp;
		tmp = last_free_bits(table[i]);
	}
	return -1;
}

int find_seq(const uint8_t *table, uint table_size, uint length)
{
	int tmp;
	uint i, start;

	if (length < 9)
		return find_seq_small(table, table_size, length);

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

void write_seq(uint8_t *table, uint index, uint length)
{
	uint tmp, start_pad, i, start_byte, end_byte;
	uint8_t tmp_byte;

	start_byte = index / 8;
	tmp = length;
	start_pad = 8 - (index % 8);
	end_byte = start_byte + ((length + (index % 8)) / 8);

	/* Start byte */
	if (tmp > start_pad) {
		tmp_byte = 0xFF >> (8 - start_pad);
		tmp -= start_pad;
	} else {
		tmp_byte = 0xFF << (8 - tmp);
		tmp_byte = tmp_byte >> (8 - start_pad);
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

bool check_seq(uint8_t *table, uint index, uint length)
{
	uint tmp, start_pad, i, start_byte, end_byte, byte_length;
	uint8_t tmp_byte, *seq;

	start_byte = index / 8;
	tmp = length;
	start_pad = 8 - (index % 8);
	end_byte = start_byte + ((length + (index % 8)) / 8);
	byte_length = end_byte - start_byte;
	seq = malloc(byte_length + 1);

	/* Start byte */
	if (tmp > start_pad) {
		tmp_byte = 0xFF >> (8 - start_pad);
		tmp -= start_pad;
	} else {
		tmp_byte = 0xFF << (8 - tmp);
		tmp_byte = tmp_byte >> (8 - start_pad);
		tmp = 0;
	}
	seq[0] = tmp_byte;

	for (i = 1; i <= byte_length; ++i) {
		if (i == byte_length) {
			tmp_byte = 0xFF << (8 - tmp);
			seq[i] = tmp_byte;
		} else {
			seq[i] = 0xFF;
			tmp -= 8;
		}
	}

	for (i = 0; i <= byte_length; ++i) {
		if (seq[i] & table[i + start_byte]) {
			free(seq);
			return false;
		}
	}

	free(seq);
	return true;
}


void delete_seq(uint8_t *table, uint index, uint length)
{
	uint tmp, start_pad, i, start_byte, end_byte;
	uint8_t tmp_byte;

	start_byte = index / 8;
	tmp = length;
	start_pad = 8 - (index % 8);
	end_byte = start_byte + ((length + (index % 8)) / 8);

	/* Start byte */
	if (tmp > start_pad) {
		tmp_byte = 0xFF << start_pad;
		tmp -= start_pad;
	} else {
		tmp_byte = 0xFF << start_pad;
		tmp = 8 - (start_pad - tmp);
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

/* Delete function that can handle a allocation table that is to big for the allocation table buffer */
/*
void delete_seq_global(struct FILE_SYSTEM *fs, uint8_t *table_buffer, uint index, uint length)
{
	uint global_index, global_end, index_start, length_start, buffer_size_bit;

	buffer_size_bit = fs->alloc_table_buffer_size * 8;
	global_index = index / buffer_size_bit;
	global_end = length / buffer_size_bit;
	global_end += global_index;

	index_start = index % buffer_size_bit;
	length_start = buffer_size_bit - index_start;

	if (length < length_start) {
		length_start = length;
	}
*/
	/* delete start sequenz *//*
	disk_read(fs->disk, table_buffer, fs->alloc_table + global_index,
		fs->alloc_table_buffer_size);
	delete_seq(table_buffer, index_start, length_start);
	disk_write(fs->disk, table_buffer, fs->alloc_table + global_index,
		fs->alloc_table_buffer_size);
	length -= length_start;

	for (++global_index; global_index < global_end; ++global_index) {
		disk_read(fs->disk, table_buffer, fs->alloc_table + global_index,
			fs->alloc_table_buffer_size);
		delete_seq(table_buffer, 0, buffer_size_bit);
		disk_write(fs->disk, table_buffer, fs->alloc_table + global_index,
			fs->alloc_table_buffer_size);
		length -= buffer_size_bit;
	}
*/
	/* delete end sequenz *//*
	if (length > 0) {
		disk_read(fs->disk, table_buffer, fs->alloc_table + global_index,
			fs->alloc_table_buffer_size);
		delete_seq(table_buffer, 0, length);
		disk_write(fs->disk, table_buffer, fs->alloc_table + global_index,
			fs->alloc_table_buffer_size);
	}
}*/

/* Delete function that can handle a allocation table that is to big for the allocation table buffer */
//TODO: Disk error handling
bool delete_seq_global(struct AT_WINDOW *win, uint index, uint length)
{
	uint global_index, global_end, index_start, length_start, buffer_size_bit;

	buffer_size_bit = win->sectors * win->sector_size * 8;
	global_index = index / buffer_size_bit;
	global_end = length / buffer_size_bit;
	global_end += global_index;

	index_start = index % buffer_size_bit;
	length_start = buffer_size_bit - index_start;

	if (length < length_start) {
		length_start = length;
	}

	//TODO: Check if the window is already at the right position
	if (!move_window(win, global_index))
		return false;
	delete_seq(win->buffer, index_start, length_start);
	length -= length_start;

	for (++global_index; global_index < global_end; ++global_index) {
		if (!move_window(win, global_index))
			return false;
		delete_seq(win->buffer, 0, buffer_size_bit);
		length -= buffer_size_bit;
	}

	/* delete end sequenz */
	if (length > 0) {
		if (!move_window(win, global_index))
			return false;
		delete_seq(win->buffer, 0, length);
	}
	if (!save_window(win))
		return false;
	return true;
}

unsigned long div_up(unsigned long dividend,
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

int cmp_INODES(const void *a, const void *b)
{
	struct INODE *tmp_a, *tmp_b;
	int ret_val;

	tmp_a = (struct INODE *) a;
	tmp_b = (struct INODE *) b;
	ret_val = ((int)tmp_a->location - (int) tmp_b->location);
	return ret_val;
}


/* TODO: Does the qsort copy the list? */
void quicksort_inodes(struct INODE *inodes, int nitems)
{
	qsort(inodes, nitems, sizeof(struct INODE), cmp_INODES);
}

/* Finds the first inode that can be deleted and returns that inode*/
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size)
{
	uint i, k, ino_cnt, *pos;
	struct INODE *inodes;

	pos = malloc(fs->inode_sec * sizeof(uint));
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, IT_BUFFER, fs->inode_sec * k, pos, &ino_cnt);
		inodes = (struct INODE *) INO_BUFFER;
		load_inode_block(fs, inodes, pos, ino_cnt, k);
		for (i = 0; i < ino_cnt; ++i) {
			if (inodes[i].size >= size) {
				if ((!inodes[i].custody) ||
					isNotValid(&inodes[i])) {
					memcpy(file, &inodes[i],
						sizeof(struct INODE));
					free(pos);
					return true;
				}
			}
		}
	}
	free(pos);
	return false;
}

bool isNotValid(struct INODE *inode)
{
		return get_time() > inode->time_to_live;
}

/* Writes a single inode onto the disk */
void write_inode(struct FILE_SYSTEM *fs, struct INODE *file)
{
	struct INODE *tmp;
	uint sec_off, ino_off;

	sec_off = file->inode_offset;
	ino_off = sec_off % fs->inode_sec;
	sec_off /= fs->inode_sec;
	sec_off += fs->inode_block;

	disk_read(fs->disk, SEC_BUFFER, sec_off, 1);

	tmp = (struct INODE *) SEC_BUFFER;
	memcpy(&tmp[ino_off], file, sizeof(struct INODE));

	disk_write(fs->disk, SEC_BUFFER, sec_off, 1);
}

/* Returns the number of inodes that are valid */
uint inodes_used(struct FILE_SYSTEM *fs)
{
	uint i, k, size, ret_val;
	uint8_t tmp;

	size = fs->inode_max / 8;

	ret_val = 0;
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	for (i = 0; i < size; ++i)
		ret_val += popcount(IT_BUFFER[i]);

	for (k = 0; k < (fs->inode_max % 8); ++k) {
		tmp = 0x80 >> k;
		if (tmp & IT_BUFFER[i])
			++ret_val;
	}

	return ret_val;
}

/* Returns the position of all inodes in a block */
void get_ino_pos(struct FILE_SYSTEM *fs, uint8_t *in_tab,
	uint offset, uint *pos, uint *ino_cnt)
{
	uint i, tmp_byte, shift, start;

	start = offset / 8;
	*ino_cnt = 0;
	shift = offset % 8;

	for (i = 0; i < fs->inode_sec; ++i) {
		tmp_byte = 0x80 >> shift;
		if (in_tab[start] & tmp_byte) {
			pos[*ino_cnt] = i;
			(*ino_cnt)++;
		}
		++shift;
		if (shift > 7) {
			shift = 0;
			++start;
		}
	}
}

/* Loads all valid inodes from a block, the functions needs the position
 of all valid inodes and in ino_cnt the number of valid inodes*/
void load_inode_block(struct FILE_SYSTEM *fs, struct INODE *buffer,
	uint *pos, uint ino_cnt, uint sec_num)
{
	uint k;
	struct INODE *tmp;

	disk_read(fs->disk, SEC_BUFFER, fs->inode_block + sec_num, 1);

	tmp = (struct INODE *) SEC_BUFFER;
	for (k = 0; k < ino_cnt; ++k)
		buffer[k] = tmp[pos[k]];

}

/* Loads all inodes into the buffer, the inodes are loaded block by block */
void load_inodes_block(struct FILE_SYSTEM *fs, struct INODE *buffer)
{
	uint i, *ino_pos, ino_cnt, offset, ino_off;

	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	offset = 0;
	ino_off = 0;
	ino_pos = malloc(fs->inode_sec * sizeof(uint32_t));

	for (i = 0; i < fs->inode_block_size; ++i) {
		get_ino_pos(fs, IT_BUFFER, offset, ino_pos, &ino_cnt);
		load_inode_block(fs, &buffer[ino_off], ino_pos, ino_cnt, i);
		ino_off += ino_cnt;
		offset += fs->inode_sec;
	}

	free(ino_pos);
}

void con32to8(uint8_t *arr, uint32_t value)
{
	arr[0] = value >> 24;
	arr[1] = (value >> 16) & 0xFF;
	arr[2] = (value >> 8) & 0xFF;
	arr[3] = value & 0xFF;
}

uint32_t con8to32(uint8_t *arr)
{
	return arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];
}


