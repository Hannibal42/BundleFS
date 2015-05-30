#include "../h/utility_test.h"

uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

bool checksum_check_test(void)
{
	uint i;
	struct INODE file;
	uint8_t *check, *tmp;

	check = malloc(64);
	tmp = malloc(192);

	file.size = 128;
	file.check_size = 64;
	checksum(data1, 128, check, 64);

	for (i = 0; i < 128; ++i)
		tmp[i] = data1[i];

	for (i = 0; i < 64; ++i)
		tmp[i + 128] = check[i];

	if (!checksum_check(tmp, &file, 64))
		return false;

	file.size = 64;
	file.check_size = 16;
	if (checksum_check(data1, &file, 16))
		return false;

	free(check);
	free(tmp);
	return true;
}

bool find_sequence_small_test(void)
{

	if (find_sequence_small(table1, 64, 10) != -1)
		return false;
	table1[0] = 0x87;
	if (find_sequence_small(table1, 64, 4) != 1)
		return false;
	table1[1] = 0x08;
	table1[2] = 0x0F;
	if (find_sequence_small(table1, 64, 5) != 13)
		return false;
	if (find_sequence_small(table1, 64, 6) != 13)
		return false;
	table1[3] = 0x00;
	table1[4] = 0x01;
	if (find_sequence_small(table1, 64, 8) != 24)
		return false;
	if (find_sequence_small(table1, 64, 8) != 24)
		return false;
	if (find_sequence_small(table1, 64, 1) != 1)
		return false;

	if (find_sequence_small(table2, 256, 10) != -1)
		return false;
	table2[0] = 0x40;
	table2[1] = 0x00;
	table2[2] = 0x00;
	table2[3] = 0x00;
	table2[4] = 0x00;
	if (find_sequence_small(table2, 256, 5) != 2)
		return false;

	return true;
}

bool popcount_test(void)
{
	uint k, count;
	uint8_t i, tmp;

	for (i = 0; i < 255; ++i) {
		count = 0;
		tmp = i;
		for (k = 128; k > 0; k /= 2) {
			if (tmp / k) {
				count += 1;
				tmp -= k;
			}
		}
		if (popcount(i) != count)
			return false;
	}
	return true;
}

bool last_free_bits_test(void)
{
	uint8_t tmp;

	tmp = 0x00;
	if (last_free_bits(tmp) != 8)
		return false;
	tmp = 0x80;
	if (last_free_bits(tmp) != 7)
		return false;
	tmp = 0xC0;
	if (last_free_bits(tmp) != 6)
		return false;
	tmp = 0xE0;
	if (last_free_bits(tmp) != 5)
		return false;
	tmp = 0xF0;
	if (last_free_bits(tmp) != 4)
		return false;
	tmp = 0x08;
	if (last_free_bits(tmp) != 3)
		return false;
	tmp = 0x8C;
	if (last_free_bits(tmp) != 2)
		return false;
	tmp = 0x1E;
	if (last_free_bits(tmp) != 1)
		return false;
	tmp = 0x2F;
	if (last_free_bits(tmp) != 0)
		return false;

	return true;
}

bool get_free_bit_test(void)
{
	uint8_t tmp;

	tmp =  0x00;
	if (get_free_bit(0, tmp) != 8)
		return false;
	if (get_free_bit(3, tmp) != 5)
		return false;
	if (get_free_bit(8, tmp) != -1)
		return false;

	tmp = 0xFF;
	if (get_free_bit(0, tmp) != 0)
		return false;
	if (get_free_bit(10, tmp) != -1)
		return false;
	if (get_free_bit(3, tmp) != 0)
		return false;

	tmp = 0x87;
	if (get_free_bit(1, tmp) != 4)
		return false;
	if (get_free_bit(2, tmp) != 3)
		return false;

	tmp = 0xF0;
	if (get_free_bit(3, tmp) != 0)
		return false;
	if (get_free_bit(4, tmp) != 4)
		return false;

	return true;
}

bool find_bit_test(void)
{
	table1[10] = 0x7F;
	if (find_bit(table1, 64) != 80)
		return false;

	table1[10] = 0x00;
	if (find_bit(table1, 64) != 80)
		return false;

	table1[9] = 0xF7;
	if (find_bit(table1, 64) != 76)
		return false;

	table2[9] = 0xF7;
	if (find_bit(table2, 256) != 76)
		return false;

	return true;
}
/* TODO: More cases */
bool find_sequence_test(void)
{

	table1[10] = 0x78;
	if (find_sequence(table1, 64, 1) != 80)
		return false;
	if (find_sequence(table1, 64, 2) != 85)
		return false;
	if (find_sequence(table1, 64, 3) != 85)
		return false;
	if (find_sequence(table1, 64, 4) != -1)
		return false;

	table1[11] = 0x7F;
	if (find_sequence(table1, 64, 4) != 85)
		return false;

	table1[4] = 0xF0;
	table1[5] = 0x00;
	table1[6] = 0x0F;
	if (find_sequence(table1, 64, 14) != 36)
		return false;
	if (find_sequence(table1, 64, 15) != 36)
		return false;
	if (find_sequence(table1, 64, 17) != -1)
		return false;

	if (find_sequence(table2_empty, 256, 256) != 0)
		return false;
	if (find_sequence(table2_empty, 256, 50000) != -1)
		return false;

	return true;
}

bool write_seq_test(void)
{
	uint i;

	write_seq(table1_empty, 0, 7);
	if (table1_empty[0] != 0xFE)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 7, 1);
	if (table1_empty[0] != 0xFF)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 8, 15);
	if (table1_empty[1] != 0xFF)
		return false;
	if (table1_empty[2] != 0xFE)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 25, 10);
	if (table1_empty[0] != 0xFF)
		return false;
	if (table1_empty[1] != 0xFF)
		return false;
	if (table1_empty[2] != 0xFE)
		return false;
	if (table1_empty[3] != 0x7F)
		return false;
	if (table1_empty[4] != 0xE0)
		return false;
	for (i = 5; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	return true;
}

bool delete_seq_test(void)
{
	uint i;

	delete_seq(table1, 0, 4);
	if (table1[0] != 0x0F)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 4, 3);
	if (table1[0] != 0x01)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 7, 10);
	if (table1[0] != 0x00)
		return false;
	if (table1[1] != 0x00)
		return false;
	if (table1[2] != 0x7F)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 0, 512);
	for (i = 0; i < 64; ++i)
		if (table1[i] != 0x00)
			return false;

	return true;
}

bool write_bit_test(void)
{

	table1 = malloc(sizeof(uint8_t) * 5);
	table1[0] = 0xFF;
	table1[1] = 0XF8;
	table1[2] = 0x00;
	table1[3] = 0xFF;
	table1[4] = 0xFF;

	write_bit(table1, 0, 0);
	if (table1[0] != 0x7F)
		return false;

	write_bit(table1, 0, 1);
	if (table1[0] != 0xFF)
		return false;

	write_bit(table1, 8, 1);
	if (table1[1] != 0xF8)
		return false;

	write_bit(table1, 8, 0);
	if (table1[1] != 0x78)
		return false;

	write_bit(table1, 18, 1);
	if (table1[2] != 0x20)
		return false;

	write_bit(table1, 39, 0);
	if (table1[4] != 0xFE)
		return false;

	return true;
}

void utility_setUp(void)
{
	uint i;

	/* Allocation table */
	table1 = malloc(64);
	table1_empty = malloc(64);
	table2 = malloc(256);
	table2_empty = malloc(256);

	for (i = 0; i < 64; ++i)
		table1[i] = 0xFF;
	for (i = 0; i < 64; ++i)
		table1_empty[i] = 0x00;
	for (i = 0; i < 256; ++i)
		table2_empty[i] = 0x00;
	for (i = 0; i < 256; ++i)
		table2[i] = 0xFF;

	/* Test Data */
	data1 = malloc(128);
	data2 = malloc(1024);
	data3 = malloc(2048);

	for (i = 0; i < 128; ++i)
		data1[i] = 0x0F;
	for (i = 0; i < 1024; ++i)
		data2[i] = 0xD0;
	for (i = 0; i < 2048; ++i)
		data3[i] = 0xEE;
}

void utility_tearDown(void)
{
	free(table1);
	free(table2);
	free(table1_empty);
	free(table2_empty);
	free(data1);
	free(data2);
	free(data3);
}

void run_utility_tests(void)
{
	uint i, length;

	bool (*tests[10]) (void);
	tests[0] = write_bit_test;
	tests[1] = find_bit_test;
	tests[2] = find_sequence_test;
	tests[3] = write_seq_test;
	tests[4] = delete_seq_test;
	tests[5] = get_free_bit_test;
	tests[6] = last_free_bits_test;
	tests[7] = popcount_test;
	tests[8] = find_sequence_small_test;
	tests[9] = checksum_check_test;

	length = sizeof(tests) / sizeof(tests[0]);
	printf("--------Start Utility tests--------\n");
	for (i = 0; i < length; ++i) {
		utility_setUp();
		if (!(*tests[i])())
			printf("Error in Test %d\n", i);
		else
			printf("Test %d succeeded\n", i);
		utility_tearDown();
	}
}
