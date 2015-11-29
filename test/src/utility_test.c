#include "utility_test.h"
#include "buffer.h"

uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;
uint8_t *buffer, *at_buffer;
struct disk *disk1;
struct FILE_SYSTEM fs;

extern int find_seq_byte(uint8_t byte, uint length);


TEST_GROUP(utility_tests);

TEST_SETUP(utility_tests)
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

	/* For global function testing */
	disk1 = malloc(sizeof(struct disk));
	/*Struct setup*/
	disk_fill(disk1, "disks/disk1.disk", 65536, 512);
	disk_create(disk1, 65536);

	buffer = malloc(65536);
	at_buffer = malloc(4096);

	disk_initialize(disk1);

	for (i = 0; i < 16 * 4096; ++i)
		buffer[i] = 0xFF;

	disk_write(disk1, buffer, 0 ,15);
	disk_shutdown(disk1);
	disk_initialize(disk1);

	fs.sector_size = 4096;
	fs.sector_count = 16;
	fs.disk = disk1;
	fs.alloc_table = 0;
	fs.alloc_table_size = 5;
	fs.alloc_table_buffer_size = 4096;
	struct AT_WINDOW *window = malloc(sizeof(struct AT_WINDOW));
	init_window(window, &fs, at_buffer);
	fs.at_win = window;
}

TEST_TEAR_DOWN(utility_tests)
{
	free(table1);
	free(table2);
	free(table1_empty);
	free(table2_empty);
	free(data1);
	free(data2);
	free(data3);
	free(buffer);
	free(fs.at_win);
	free(at_buffer);
	free(disk1);
}

TEST(utility_tests, find_seq_small_test)
{

	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 10), -1);
	table1[0] = 0x87;
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 4), 1);
	table1[1] = 0x08;
	table1[2] = 0x0F;
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 5), 13);
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 6), 13);
	table1[3] = 0x00;
	table1[4] = 0x01;
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 8), 24);
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 8), 24);
	TEST_ASSERT_EQUAL_INT(find_seq_small(table1, 64, 1), 1);

	TEST_ASSERT_EQUAL_INT(find_seq_small(table2, 256, 10), -1);
	table2[0] = 0x40;
	table2[1] = 0x00;
	table2[2] = 0x00;
	table2[3] = 0x00;
	table2[4] = 0x00;
	TEST_ASSERT_EQUAL_INT(find_seq_small(table2, 256, 5), 2);

	table2[0] = 0x07;
	TEST_ASSERT_EQUAL_INT(find_seq_small(table2, 1, 5), 0);
}

TEST(utility_tests, popcount_test)
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
		TEST_ASSERT_EQUAL_INT(popcount(i), count);
	}
}

TEST(utility_tests, last_free_bits_test)
{
	uint8_t tmp;

	tmp = 0x00;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 8);
	tmp = 0x80;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 7);
	tmp = 0xC0;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 6);
	tmp = 0xE0;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 5);
	tmp = 0xF0;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 4);
	tmp = 0x08;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 3);
	tmp = 0x8C;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 2);
	tmp = 0x1E;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 1);
	tmp = 0x2F;
	TEST_ASSERT_EQUAL_INT(last_free_bits(tmp), 0);
}

TEST(utility_tests, get_free_bit_test)
{
	uint8_t tmp;

	tmp =  0x00;
	TEST_ASSERT_EQUAL_INT(get_free_bit(0, tmp), 8);
	TEST_ASSERT_EQUAL_INT(get_free_bit(3, tmp), 5);
	TEST_ASSERT_EQUAL_INT(get_free_bit(8, tmp), -1);

	tmp = 0xFF;
	TEST_ASSERT_EQUAL_INT(get_free_bit(0, tmp), 0);
	TEST_ASSERT_EQUAL_INT(get_free_bit(10, tmp), -1);
	TEST_ASSERT_EQUAL_INT(get_free_bit(3, tmp), 0);

	tmp = 0x87;
	TEST_ASSERT_EQUAL_INT(get_free_bit(1, tmp), 4);
	TEST_ASSERT_EQUAL_INT(get_free_bit(2, tmp), 3);

	tmp = 0xF0;
	TEST_ASSERT_EQUAL_INT(get_free_bit(3, tmp), 0);
	TEST_ASSERT_EQUAL_INT(get_free_bit(4, tmp), 4);
}

TEST(utility_tests, find_bit_test)
{
	table1[10] = 0x7F;
	TEST_ASSERT_EQUAL_INT(find_bit(table1, 64), 80);

	table1[10] = 0x00;
	TEST_ASSERT_EQUAL_INT(find_bit(table1, 64), 80);

	table1[9] = 0xF7;
	TEST_ASSERT_EQUAL_INT(find_bit(table1, 64), 76);

	table2[9] = 0xF7;
	TEST_ASSERT_EQUAL_INT(find_bit(table2, 256), 76);
}

TEST(utility_tests, find_seq_test)
{

	table1[10] = 0x78;
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 1), 80);
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 2), 85);
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 3), 85);
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 4), -1);

	table1[11] = 0x7F;
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 4), 85);

	table1[4] = 0xF0;
	table1[5] = 0x00;
	table1[6] = 0x0F;
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 14), 36);
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 15), 36);
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 17), -1);

	TEST_ASSERT_EQUAL_INT(find_seq(table2_empty, 256, 256), 0);
	TEST_ASSERT_EQUAL_INT(find_seq(table2_empty, 256, 50000), -1);

	table1[4] = 0x80;
	table1[5] = 0x00;
	table1[6] = 0x10;
	TEST_ASSERT_EQUAL_INT(find_seq(table1, 64, 15), 33);
}

TEST(utility_tests, write_seq_test)
{
	uint i;

	write_seq(table1_empty, 0, 7);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[0], 0xFE);
	for (i = 1; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1_empty[i], 0x00);

	write_seq(table1_empty, 7, 1);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[0], 0xFF);
	for (i = 1; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1_empty[i], 0x00);

	write_seq(table1_empty, 8, 15);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[1], 0xFF);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[2], 0xFE);
	for (i = 3; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1_empty[i], 0x00);

	write_seq(table1_empty, 25, 10);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[0], 0xFF);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[1], 0xFF);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[2], 0xFE);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[3], 0x7F);
	TEST_ASSERT_EQUAL_HEX8(table1_empty[4], 0xE0);
	for (i = 5; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1_empty[i], 0x00);
}

TEST(utility_tests, delete_seq_test)
{
	uint i;

	delete_seq(table1, 0, 4);
	TEST_ASSERT_EQUAL_HEX8(table1[0], 0x0F);
	for (i = 1; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1[i], 0xFF);

	delete_seq(table1, 4, 3);
	TEST_ASSERT_EQUAL_HEX8(table1[0], 0x01);
	for (i = 1; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1[i], 0xFF);

	delete_seq(table1, 7, 10);
	TEST_ASSERT_EQUAL_HEX8(table1[0], 0x00);
	TEST_ASSERT_EQUAL_HEX8(table1[1], 0x00);
	TEST_ASSERT_EQUAL_HEX8(table1[2], 0x7F);
	for (i = 3; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1[i], 0xFF);

	delete_seq(table1, 0, 512);
	for (i = 0; i < 64; ++i)
		TEST_ASSERT_EQUAL_HEX8(table1[i], 0x00);
}

TEST(utility_tests, write_bit_test)
{
	table1[0] = 0xFF;
	table1[1] = 0XF8;
	table1[2] = 0x00;
	table1[3] = 0xFF;
	table1[4] = 0xFF;

	write_bit(table1, 0, 0);
	TEST_ASSERT_EQUAL_HEX8(table1[0], 0x7F);

	write_bit(table1, 0, 1);
	TEST_ASSERT_EQUAL_HEX8(table1[0], 0xFF);

	write_bit(table1, 8, 1);
	TEST_ASSERT_EQUAL_HEX8(table1[1], 0xF8);

	write_bit(table1, 8, 0);
	TEST_ASSERT_EQUAL_HEX8(table1[1], 0x78);

	write_bit(table1, 18, 1);
	TEST_ASSERT_EQUAL_HEX8(table1[2], 0x20);

	write_bit(table1, 39, 0);
	TEST_ASSERT_EQUAL_HEX8(table1[4], 0xFE);
}

TEST(utility_tests, quick_sort_inodes_test)
{
	uint i;
	struct INODE a, b, c, d, e, f, g, h;

	a.location = 20;
	b.location = 20;
	c.location = 21;
	d.location = 300;
	e.location = 570;
	f.location = 3245;
	g.location = 324567;
	h.location = 334567856;

	struct INODE solution[8] = {a, b, c, d, e, f, g, h};
	struct INODE inodes1[8] = {h, g, d, a, b, e, c, f};
	struct INODE inodes2[8] = {c, d, e, f, a, g, h, b};

	quicksort_inodes(inodes1, 8);
	quicksort_inodes(inodes2, 8);

	for (i = 0; i < 8; ++i)
		TEST_ASSERT_EQUAL_UINT(solution[i].location,
		inodes1[i].location);

	for (i = 0; i < 8; ++i)
		TEST_ASSERT_EQUAL_UINT(solution[i].location,
		inodes2[i].location);
}

TEST(utility_tests, find_seq_byte_test)
{
	TEST_ASSERT_EQUAL(0, find_seq_byte(0x03, 6));
	TEST_ASSERT_EQUAL(1, find_seq_byte(0x80, 6));
	TEST_ASSERT_EQUAL(0, find_seq_byte(0x07, 5));
}

TEST(utility_tests, check_seq_test)
{
	table1[11] = 0xC0;
	TEST_ASSERT_TRUE(check_seq(table1, 90, 6));
	TEST_ASSERT_TRUE(check_seq(table1, 90, 6));
	TEST_ASSERT_FALSE(check_seq(table1, 90, 7));
	TEST_ASSERT_TRUE(check_seq(table1, 91, 5));

	table1[12] = 0x01;
	TEST_ASSERT_TRUE(check_seq(table1, 90, 7));
	TEST_ASSERT_TRUE(check_seq(table1, 90, 13));
	TEST_ASSERT_FALSE(check_seq(table1, 90, 14));

	table1[12] = 0x00;
	table1[13] = 0x70;
	TEST_ASSERT_TRUE(check_seq(table1, 95, 6));
	TEST_ASSERT_TRUE(check_seq(table1, 90, 14));
	TEST_ASSERT_TRUE(check_seq(table1, 90, 15));
	TEST_ASSERT_FALSE(check_seq(table1, 90, 16));
	TEST_ASSERT_TRUE(check_seq(table1, 96, 9));

	table1[14] = 0xEF;
	TEST_ASSERT_TRUE(check_seq(table1, 115, 1));
	TEST_ASSERT_TRUE(check_seq(table1, 115, 0));
	TEST_ASSERT_TRUE(check_seq(table1, 160, 0));
}

TEST(utility_tests, calc_fake_crc_test)
{
	uint8_t tmp;

	reset_fake_crc();
	tmp = calc_fake_crc(0x00);
	TEST_ASSERT_EQUAL_HEX8(0x00, tmp);

	calc_fake_crc(0x70);
	tmp = calc_fake_crc(0xAA);
	TEST_ASSERT_EQUAL_HEX8(0xDA, tmp);
	tmp = calc_fake_crc(0xFF);
	TEST_ASSERT_EQUAL_HEX8(0x25, tmp);
	reset_fake_crc();
	tmp = calc_fake_crc(0x00);
	TEST_ASSERT_EQUAL_HEX8(0x00, tmp);
}

TEST(utility_tests, get_ino_pos_test)
{
	struct FILE_SYSTEM fs;
	uint8_t tmp[4];
	uint *pos, ino_cnt;

	tmp[0] = 0xF0; tmp[1] = 0xFF;
	tmp[2] = 0xF8; tmp[3] = 0x8F;

	fs.inode_sec = 5;
	pos = malloc(5 * sizeof(uint));

	get_ino_pos(&fs, tmp, 20, pos, &ino_cnt);
	TEST_ASSERT_EQUAL_UINT(2, ino_cnt);
	TEST_ASSERT_EQUAL_UINT(0, pos[0]);
	TEST_ASSERT_EQUAL_UINT(4, pos[1]);

	get_ino_pos(&fs, tmp, 25, pos, &ino_cnt);
	TEST_ASSERT_EQUAL_UINT(2, ino_cnt);
	TEST_ASSERT_EQUAL_UINT(3, pos[0]);
	TEST_ASSERT_EQUAL_UINT(4, pos[1]);

	fs.inode_sec = 7;
	get_ino_pos(&fs, tmp, 25, pos, &ino_cnt);
	TEST_ASSERT_EQUAL_UINT(4, ino_cnt);
	TEST_ASSERT_EQUAL_UINT(3, pos[0]);
	TEST_ASSERT_EQUAL_UINT(4, pos[1]);
	TEST_ASSERT_EQUAL_UINT(5, pos[2]);
	TEST_ASSERT_EQUAL_UINT(6, pos[3]);

	get_ino_pos(&fs, tmp, 0, pos, &ino_cnt);
	TEST_ASSERT_EQUAL_UINT(4, ino_cnt);
	TEST_ASSERT_EQUAL_UINT(0, pos[0]);
	TEST_ASSERT_EQUAL_UINT(1, pos[1]);
	TEST_ASSERT_EQUAL_UINT(2, pos[2]);
	TEST_ASSERT_EQUAL_UINT(3, pos[3]);

	free(pos);
}

TEST(utility_tests, delete_seq_global_test)
{
	int i;

	TEST_ASSERT_TRUE(delete_seq_global(fs.at_win, 40, 10));

	disk_read(disk1, buffer, 0, 16);

	TEST_ASSERT_EQUAL_HEX8(0x00, buffer[5]);
	TEST_ASSERT_EQUAL_HEX8(0x3F, buffer[6]);

	TEST_ASSERT_TRUE(delete_seq_global(fs.at_win, 5000 * 8, 10));

	
	disk_read(disk1, buffer, 0, 16);

	TEST_ASSERT_EQUAL_HEX8(0x00, buffer[5000]);
	TEST_ASSERT_EQUAL_HEX8(0x3F, buffer[5001]);
	

}

TEST_GROUP_RUNNER(utility_tests)
{
	RUN_TEST_CASE(utility_tests, get_ino_pos_test);
	RUN_TEST_CASE(utility_tests, write_bit_test);
	RUN_TEST_CASE(utility_tests, write_seq_test);
	RUN_TEST_CASE(utility_tests, delete_seq_test);
	RUN_TEST_CASE(utility_tests, find_seq_test);
	RUN_TEST_CASE(utility_tests, find_bit_test);
	RUN_TEST_CASE(utility_tests, get_free_bit_test);
	RUN_TEST_CASE(utility_tests, last_free_bits_test);
	RUN_TEST_CASE(utility_tests, popcount_test);
	RUN_TEST_CASE(utility_tests, find_seq_small_test);
	RUN_TEST_CASE(utility_tests, quick_sort_inodes_test);
	RUN_TEST_CASE(utility_tests, find_seq_byte_test);
	RUN_TEST_CASE(utility_tests, check_seq_test);
	RUN_TEST_CASE(utility_tests, calc_fake_crc_test);
	RUN_TEST_CASE(utility_tests, delete_seq_global_test);
}
