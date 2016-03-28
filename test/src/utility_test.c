#include "utility_test.h"

#include "../../include/window_buffer.h"
#include "file_system.h"

uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;
uint8_t *buffer, *at_buffer;
struct DISK *disk1;
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
	disk1 = malloc(sizeof(struct DISK));
	/*Struct setup*/
	disk_fill(disk1, "disks/disk1.disk", 65536, 512);
	disk_create(disk1, 65536);
	disk1->sector_block_mapping = 8;

	buffer = malloc(65536);
	at_buffer = malloc(4096);

	disk_initialize(disk1);

	for (i = 0; i < 65536; ++i)
		buffer[i] = 0xFF;
	disk_write(disk1, buffer, 0, 16);

	disk_shutdown(disk1);
	disk_initialize(disk1);

	fs.sector_size = 4096;
	fs.sector_count = 16;
	fs.disk = disk1;
	fs.alloc_table = 0;
	fs.alloc_table_size = 10;
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
	/* Checks if the delete makes bit toggle, or sets them to 0 */
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
	
	/* Delete more than one sector */
	TEST_ASSERT_TRUE(delete_seq_global(fs.at_win, 5, 4900 * 8));

	disk_read(disk1, buffer, 0, 16);

	TEST_ASSERT_EQUAL_HEX8(0xF8, buffer[0]);
	for (i = 1; i < 4900; ++i)
		TEST_ASSERT_EQUAL_HEX8(0x00, buffer[i]);
	TEST_ASSERT_EQUAL_HEX8(0x07, buffer[4900]);

	/* Delete a lot sectors */
	TEST_ASSERT_TRUE(delete_seq_global(fs.at_win, 5050 * 8 + 3, 4096 * 8 * 5));

	disk_read(disk1, buffer, 0, 16);

	TEST_ASSERT_EQUAL_HEX8(0xE0, buffer[5050]);
	for (i = 5051; i < 4096 * 5 + 5050; ++i){
		TEST_ASSERT_EQUAL_HEX8(0x00, buffer[i]);
	}

	/* Delete first byte */
	TEST_ASSERT_TRUE(delete_seq_global(fs.at_win, 0, 8));
	disk_read(disk1, buffer, 0, 16);

	TEST_ASSERT_EQUAL_HEX8(0x00, buffer[0]);
}

TEST(utility_tests, write_seq_global_test) {
	int i;

	disk_read(disk1, buffer, 0, 16);
	for (i = 0; i < 4096; ++i)
		buffer[i] = 0x00;
	disk_write(disk1, buffer, 1, 16);

	/* Small sequence  */
	TEST_ASSERT_TRUE(write_seq_global(fs.at_win, 0, 10));
	disk_read(disk1, buffer, 0, 16);
	TEST_ASSERT_EQUAL_HEX8(0xFF, buffer[0]);
	TEST_ASSERT_EQUAL_HEX8(0x00, buffer[2]);

}

TEST(utility_tests, find_seq_global_1_test) {
	uint i;
	/* Smallest sequence */
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 1, &i));
	buffer[0] = 0x7F;
	disk_write(disk1, buffer, 0, 16);
	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 1, &i));
	TEST_ASSERT_EQUAL_INT(0, i);

	/* Small sequence */
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 40, &i));

	disk_read(disk1, buffer, 0, 16);

	buffer[0] = 0x00;
	buffer[1] = 0x00;

	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 10, &i));
	TEST_ASSERT_EQUAL_INT(0, i);

	/* Sequence over 2 sectors */
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 20, &i));

	buffer[4095] = 0x00;
	buffer[4096] = 0x00;
	buffer[4097] = 0x00;
	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 20, &i));
	TEST_ASSERT_EQUAL(32760, i);

	/* Sequence at the end of the allocation table  */
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 32, &i));


	/* TODO: What the fuck is not working with this?
	Weird bug
	buffer[4096 * 10 -1] = 0x00;
	buffer[4096 * 10 -2] = 0x00;
	buffer[4096 * 10 -3] = 0x00;
	buffer[4096 * 10 -4] = 0x00;
	disk_write(disk1, buffer, 0, 16);
	*/
	TEST_ASSERT_TRUE(move_window(fs.at_win, 9));
	fs.at_win->buffer[4095] = 0x00;
	fs.at_win->buffer[4094] = 0x00;
	fs.at_win->buffer[4093] = 0x00;
	fs.at_win->buffer[4092] = 0x00;
	TEST_ASSERT_TRUE(save_window(fs.at_win));

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 32, &i));
	TEST_ASSERT_EQUAL(327648, i);
}

/* Test some sequences that are longer than one buffer window */
TEST(utility_tests, find_seq_global_2_test) {
	int i, tmp;
	uint k = 0;

	/* Window buffer size +1 */
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 4097 * 8, &k));
	for (i = 0; i < 4097; ++i)
		buffer[i] = 0x00;

	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 4097 * 8, &k));
	TEST_ASSERT_EQUAL_UINT(0, k);

	/* Sequence not starting at zero*/
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, 5000 * 8, &k));
	buffer[0] = 0xFF;
	for (i = 1; i < 5001; ++i)
		buffer[i] = 0x00;
	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 5000 * 8, &k));
	TEST_ASSERT_EQUAL_INT(8, k);

	/* 4 sector sequence*/
	tmp = 4096 * 4;
	TEST_ASSERT_FALSE(find_seq_global(fs.at_win, tmp * 8, &k));

	for (i = 0; i < tmp; ++i)
		buffer[i] = 0x00;

	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, tmp * 8, &k));
	TEST_ASSERT_EQUAL_INT(0, k);

	/*Start in the second sector*/
	for (i = 0; i < 4096; ++i)
		buffer[i] = 0xFF;

	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 5000, &k));
	TEST_ASSERT_EQUAL_INT(4096 * 8, k);

	/* All sectors */
	for (i = 0; i < 4096 * 10; ++i)
		buffer[i] = 0;

	disk_write(disk1, buffer, 0, 16);

	TEST_ASSERT_TRUE(find_seq_global(fs.at_win, 4096 * 10 * 8, &k));
	TEST_ASSERT_EQUAL_INT(0, k);
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
	RUN_TEST_CASE(utility_tests, write_seq_global_test);
	RUN_TEST_CASE(utility_tests, find_seq_global_1_test);
	RUN_TEST_CASE(utility_tests, find_seq_global_2_test);
}
