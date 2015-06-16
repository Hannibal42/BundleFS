#include "system_tests.h"

struct disk disks[8];
uint8_t *data[4];

TEST_GROUP(system_tests);

TEST_SETUP(system_tests)
{
	uint i;

	disk_fill(&disks[0], "disks/disk0.disk", 1024, 64);
	disk_create(&disks[0], 1024);
	disk_fill(&disks[1], "disks/disk1.disk", 4096, 64);
	disk_create(&disks[1], 4096);
	disk_fill(&disks[2], "disks/disk2.disk", 4096, 128);
	disk_create(&disks[2], 4096);
	disk_fill(&disks[3], "disks/disk3.disk", 16384, 64);
	disk_create(&disks[3], 16384);
	disk_fill(&disks[4], "disks/disk4.disk", 16384, 256);
	disk_create(&disks[4], 16384);
	disk_fill(&disks[5], "disks/disk5.disk", 262144, 512);
	disk_create(&disks[5], 262144);
	disk_fill(&disks[6], "disks/disk6.disk", 524288, 512);
	disk_create(&disks[6], 524288);
	disk_fill(&disks[7], "disks/disk7.disk", 1048576, 512);
	disk_create(&disks[7], 1048576);

	for (i = 0; i < 8; ++i) {
		disk_initialize(&disks[i]);
		fs_mkfs(&disks[i]);
		disk_shutdown(&disks[i]);
		disk_initialize(&disks[i]);
	}

	data[0] = malloc(100);
	for (i = 0; i < 100; ++i)
		data[0][i] = 0xEA;

	data[1] = malloc(512);
	for (i = 0; i < 512; ++i)
		data[1][i] = 0xF0;

	data[2] = malloc(5000);
	for (i = 0; i < 5000; ++i)
		data[2][i] = 0xAB;

	data[3] = malloc(100000);
	for (i = 0; i < 100000; ++i)
		data[3][i] = 0xCC;

	/* TODO: Make random data */
}

TEST_TEAR_DOWN(system_tests)
{
	uint i;

	for (i = 0; i < 8; ++i)
		disk_shutdown(&disks[i]);
	for (i = 0; i < 4; ++i)
			free(data[i]);
}

TEST(system_tests, overflow_disk_test)
{
	uint i, k;
	struct FILE_SYSTEM fs;
	struct INODE tmp;

	for (k = 4; k < 8; ++k) {
		TEST_ASSERT_EQUAL(FS_OK, fs_mount(&disks[k], &fs));
		for (i = 0; i < 100000; ++i)
			if (fs_create(&fs, &tmp, 100, 100, false) != FS_OK) {
				delete_invalid_inodes(&fs);
				defragment(&fs);
				TEST_ASSERT_EQUAL(FS_OK,
					fs_create(&fs, &tmp, 100, 100, false));
			}
	}
}

TEST(system_tests, mul_read_writes_test)
{
	uint i, k, r;
	struct FILE_SYSTEM fs;
	struct INODE tmp;
	uint8_t *buffer;

	buffer = malloc(100);

	for (k = 0; k < 8; ++k) {
		TEST_ASSERT_EQUAL(FS_OK, fs_mount(&disks[k], &fs));
		for (i = 0; i < 1000; ++i) {
			TEST_ASSERT_EQUAL(FS_OK, fs_create(&fs, &tmp,
				100, 100, false));
			TEST_ASSERT_EQUAL(FS_OK, fs_write(&fs, &tmp,
				(char *) data[0]));
			TEST_ASSERT_EQUAL(FS_OK, fs_read(&fs, &tmp,
				(char *) buffer, 100));
			for (r = 0; r < 100; ++r)
				TEST_ASSERT_EQUAL_HEX8(data[0][r], buffer[r]);
			TEST_ASSERT_EQUAL(FS_OK, fs_delete(&fs, &tmp));
		}
	}

	free(buffer);
}

TEST_GROUP_RUNNER(system_tests)
{
	RUN_TEST_CASE(system_tests, overflow_disk_test);
	RUN_TEST_CASE(system_tests, mul_read_writes_test);
}
