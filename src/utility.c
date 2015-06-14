#include "utility.h"


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
int find_sequence_small(const uint8_t *table, uint table_size, uint length)
{
	uint i;
	int tmp;

	tmp = find_sequence_byte(table[0], length);
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
		tmp = find_sequence_byte(table[i], length);
		if (tmp > 0)
			return i * 8 + tmp;
		tmp = last_free_bits(table[i]);
	}
	return -1;
}

int find_sequence(const uint8_t *table, uint table_size, uint length)
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

void write_seq(uint8_t *table, uint index, uint length)
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

void delete_seq(uint8_t *table, uint index, uint length)
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

/* TODO: Change this function for upcn*/
uint checksum_size(uint size)
{
	return size / 8;
}

/* TODO: Change this function for upcn*/
void checksum(const uint8_t *buffer, uint length, uint8_t *result, uint size)
{
	uint i;
	uint8_t *tmp;

	tmp = malloc(length);
	memcpy(tmp, buffer, length);

	while (length > size) {
		for (i = 0; i < (length - 1); ++i)
				tmp[i] = tmp[i] ^ tmp[i + 1];
		length /= 2;
	}

	memcpy(result, tmp, size);

	free(tmp);
}

bool checksum_check(const uint8_t *buffer, const struct INODE *file,
	uint sector_size)
{
	uint i, sector_count_file, sector_count_check, fbc,
	 cbc;
	uint8_t *tmp;

	sector_count_file = div_up(file->size, sector_size);
	fbc = sector_count_file * sector_size;
	sector_count_check = div_up(file->check_size, sector_size);
	cbc = sector_count_check * sector_size;

	tmp = malloc(cbc);
	checksum(buffer, fbc, tmp, file->check_size);

	for (i = 0; i < file->check_size; ++i) {
		if (buffer[i + fbc] != tmp[i]) {
			free(tmp);
			return false;
		}
	}

	free(tmp);
	return true;
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

void quicksort_inodes(struct INODE *inodes, int nitems)
{
	qsort(inodes, nitems, sizeof(struct INODE), cmp_INODES);
}
