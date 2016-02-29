#include "../include/window_buffer_test.h"

struct AT_WINDOW *window;
struct FILE_SYSTEM *fs;
struct DISK *disk;
uint8_t *buffer;

#define DISK_SEC_SIZE 4096
#define DISK_SIZE DISK_SEC_SIZE * 6

TEST_GROUP(buffer_tests);

TEST_SETUP(buffer_tests)
{
	buffer = malloc(DISK_SEC_SIZE);
	fs = malloc(sizeof(struct FILE_SYSTEM));
	disk = malloc(sizeof(struct DISK));
	window = malloc(sizeof(struct AT_WINDOW));

	disk_fill(disk, "disks/disk1.disk", DISK_SIZE, DISK_SEC_SIZE);
	disk->sector_block_mapping = 1;

	disk_create(disk, DISK_SIZE);
	disk_initialize(disk);
	disk_shutdown(disk);
	disk_initialize(disk);

	fs->sector_size = DISK_SEC_SIZE;
	fs->sector_count = 6;
	fs->disk = disk;
	fs->alloc_table = 1;
	fs->alloc_table_size = 5;
	fs->alloc_table_buffer_size = 4096; 
	fs->at_win = window;
}

TEST_TEAR_DOWN(buffer_tests)
{
	free(buffer);
	free(window);
	free(fs);
	free(disk);
}

/*
 * Tests if the window functions still work, if the
 * block size of the disk is different from the block size
 * of the file system.
*/
TEST(buffer_tests, different_block_sizes)
{
	struct DISK *disk2;
	int i;

	disk2 = malloc(sizeof(struct DISK));
	disk_fill(disk2, "disks/disk2.disk", 512 * 1000, 512);
	disk_create(disk2, 512 * 1000);
	disk_initialize(disk2);
	disk2->sector_block_mapping = 8;

	fs->sector_count = 10;
	fs->disk = disk2;

	init_window(window, fs, buffer);
	for (i = 0; i < DISK_SEC_SIZE; ++i) {
		buffer[i] = 0xFF;
	}

	TEST_ASSERT_TRUE(move_window(window, 1));
	TEST_ASSERT_TRUE(move_window(window, 0));

	TEST_ASSERT_EQUAL_HEX(0xFF, buffer[514]);


	free(disk2);
}

TEST(buffer_tests, init_window_test)
{
	TEST_ASSERT_TRUE(init_window(window, fs, buffer));

	TEST_ASSERT_TRUE(window->isValid);
	TEST_ASSERT_TRUE(buffer == window->buffer);
	TEST_ASSERT_EQUAL(1, window->global_index);
	TEST_ASSERT_EQUAL(5, window->global_end);
	TEST_ASSERT_EQUAL(1, window->global_start);
	TEST_ASSERT_EQUAL(1, window->sectors);
	TEST_ASSERT_EQUAL(4096, window->sector_size);
	TEST_ASSERT_TRUE(fs->disk == window->disk);

}

TEST(buffer_tests, move_window_test)
{
	init_window(window, fs, buffer);
	TEST_ASSERT_FALSE(move_window(window, 100));
	window->buffer[0] = 0XFF;
	window->buffer[1] = 0XFF;
	TEST_ASSERT_TRUE(move_window(window, 1));
	window->buffer[0] = 0x0F;
	window->buffer[1] = 0x1F;
	TEST_ASSERT_EQUAL(2, window->global_index);
	TEST_ASSERT_TRUE(move_window(window, 0));
	TEST_ASSERT_EQUAL(1, window->global_index);
	TEST_ASSERT_EQUAL(0xFF, window->buffer[0]);
	TEST_ASSERT_EQUAL(0xFF, window->buffer[1]);

	TEST_ASSERT_TRUE(move_window(window, 1));
	TEST_ASSERT_EQUAL(0x0F, window->buffer[0]);
	TEST_ASSERT_EQUAL(0x1F, window->buffer[1]);

	TEST_ASSERT_FALSE(move_window(window, -1));
}

TEST(buffer_tests, save_window_test)
{
	init_window(window, fs, buffer);
	window->buffer[0] = 0XFF;
	window->buffer[1] = 0XFF;

	TEST_ASSERT_TRUE(save_window(window));

	disk_read(disk, buffer, 1, 1);

	TEST_ASSERT_EQUAL(0xFF, window->buffer[0]);
	TEST_ASSERT_EQUAL(0xFF, window->buffer[1]);
}

TEST_GROUP_RUNNER(buffer_tests)
{
	RUN_TEST_CASE(buffer_tests, init_window_test);
	RUN_TEST_CASE(buffer_tests, move_window_test);
	RUN_TEST_CASE(buffer_tests, save_window_test);
	RUN_TEST_CASE(buffer_tests, different_block_sizes);
}
