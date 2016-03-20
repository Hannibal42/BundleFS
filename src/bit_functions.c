#include "../include/bit_functions.h"


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

/* Length must be < 9 for this, and the table must be > 0*/
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

	/* delete end sequence */
	if (length > 0) {
		if (!move_window(win, global_index))
			return false;
		delete_seq(win->buffer, 0, length);
	}
	if (!save_window(win))
		return false;
	return true;
}

// TODO: make function for write and delete in one impl
bool write_seq_global(struct AT_WINDOW *win, uint index, uint length)
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

	if (!move_window(win, global_index))
		return false;
	write_seq(win->buffer, index_start, length_start);
	length -= length_start;

	for (++global_index; global_index < global_end; ++global_index) {
		if (!move_window(win, global_index))
			return false;
		write_seq(win->buffer, 0, buffer_size_bit);
		length -= buffer_size_bit;
	}

	/* delete end sequence */
	if (length > 0) {
		if (!move_window(win, global_index))
			return false;
		write_seq(win->buffer, 0, length);
	}
	if (!save_window(win))
		return false;
	return true;
}


/* Checks if the addressed sequence of bit is zero  */
bool check_seq_global(struct AT_WINDOW *win, uint index, uint length)
{
	uint global_index, global_end, start_index, start_length, buffer_size_bit;

	buffer_size_bit = win->sectors * win->sector_size * 8;
	global_index = index / buffer_size_bit;
	global_end = length / buffer_size_bit;
	global_end += global_index;

	start_index = index % buffer_size_bit;
	start_length = buffer_size_bit - start_index;

	if (length < start_length) {
		start_length = length;
	}

	if (!move_window(win, global_index))
		return false;
	/* Check the start of the sequence */
	if (!check_seq(win->buffer, start_index, start_length))
		return false;
	length -= start_length;

	/* Check the middle of the sequence */
	for (++global_index; global_index < global_end; ++global_index) {
		if (!move_window(win, global_index))
			return false;
		if (!check_seq(win->buffer, 0, buffer_size_bit))
			return false;
		length -= buffer_size_bit;
	}

	/* Check the end of the sequence */
	if (length > 0) {
		if (!move_window(win, global_index))
			return false;
		if (!check_seq(win->buffer, 0, length))
			return false;
	}
	return true;
}

/* Finds the largest sequence in the allocation table that is free */
bool find_largest_seq_global(struct AT_WINDOW *win, uint *length, uint *index)
{

	return false;
}


/*
bool find_seq_global(struct AT_WINDOW *win, uint table_size, uint length, uint *index)
{
	uint global_index, global_end, index_start, length_start, buffer_size_bit, ret_index, bit_count;

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


	for (++global_index; global_index < global_end; ++global_index) {
		if (!move_window(win, global_index))
			return false;
		write_seq(win->buffer, 0, buffer_size_bit);
		length -= buffer_size_bit;
	}

	if (length > 0) {
		if (!move_window(win, global_index))
			return false;
		write_seq(win->buffer, 0, length);
	}
	if (!save_window(win))
		return false;
	return true;
}*/

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


