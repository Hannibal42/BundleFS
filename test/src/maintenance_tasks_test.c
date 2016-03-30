#include "../include/maintenance_tasks_test.h"

#include <unistd.h>
#include <sys/time.h>

struct DISK *disk1;
struct FILE_SYSTEM fs1;
struct AT_WINDOW win;
struct INODE in1, in2, in3, in4;
uint8_t *data1, *data2;

extern uint inodes_used(struct FILE_SYSTEM *fs);

TEST_GROUP(taks_tests);


TEST_SETUP(tasks_tests)
{
	uint i;

	disk1 = malloc(sizeof(struct DISK));
	disk_fill(disk1, "disks/disk1.disk", 4096, 64);
	disk_create(disk1, 4096);
	disk_initialize(disk1);
	fs_mkfs(disk1, 64);
	disk_shutdown(disk1);
	disk_initialize(disk1);

	/* Inode filling */
	in1.size = 10;
	in1.location = 0;
	in1.inode_offset = 0;
	in1.custody = false;
	in1.time_to_live = 100;

	in2.size = 1;
	in2.location = 10;
	in2.inode_offset = 1;
	in2.custody = true;
	in2.time_to_live = 100;

	in3.size = 2;
	in3.location = 11;
	in3.inode_offset = 2;
	in3.custody = true;
	in3.time_to_live = 100;

	in4.size = 2;
	in4.location = 11;
	in4.inode_offset = 2;
	in4.custody = true;
	in4.time_to_live = (uint) time(NULL) + 500;

	/* Test Data */
	data1 = malloc(128);
	data2 = malloc(1024);

	for (i = 0; i < 128; ++i)
		data1[i] = 0x0F;
	for (i = 0; i < 1024; ++i)
		data2[i] = 0xD0;

}

TEST_TEAR_DOWN(tasks_tests)
{
	disk_shutdown(disk1);
	free(disk1);
	free(data1);
	free(data2);
}

TEST(tasks_tests, defragment_test)
{
	uint i, k;
	struct INODE inodes[3] = {in1, in2, in3};
	struct INODE tmp;

	fs_mount(disk1, &fs1, &win);

	for (i = 0; i < 3; ++i)
		fs_create(&fs1, &inodes[i], fs1.sector_size, 1, true);

	fs_delete(&fs1, &inodes[1]);

	defragment(&fs1);

	fs_open(&fs1, inodes[0].inode_offset, &tmp);

	TEST_ASSERT_EQUAL(tmp.location, 62);
	TEST_ASSERT_EQUAL(tmp.inode_offset, 0);
	fs_delete(&fs1, &tmp);

	fs_open(&fs1, inodes[2].inode_offset, &tmp);
	TEST_ASSERT_EQUAL(tmp.location, 60);
	TEST_ASSERT_EQUAL(tmp.inode_offset, 2);
	fs_delete(&fs1, &tmp);

	for (i = 0; i < 8; ++i) {
		tmp.inode_offset = i;
		fs_create(&fs1, &tmp, fs1.sector_size, 1, true);
	}

	for (i = 0; i < 4; ++i) {
		fs_open(&fs1, i * 2, &tmp);
		fs_delete(&fs1, &tmp);
	}

	defragment(&fs1);

	k = 62;
	for (i = 0; i < 4; ++i) {
		fs_open(&fs1, 2 * i + 1, &tmp);
		TEST_ASSERT_EQUAL(tmp.location, k);
		TEST_ASSERT_EQUAL(tmp.inode_offset, 2 * i + 1);
		k -= 2;
	}

}

TEST(tasks_tests, defragment_test2)
{
	uint i, k;
	struct INODE tmp;
	struct INODE inodes[8];
	uint8_t *buf;

	fs_mount(disk1, &fs1, &win);

	for (i = 0; i < 3; ++i) {
		fs_create(&fs1, &tmp, fs1.sector_size, 1, true);
		inodes[i] = tmp;
		fs_write(&fs1, &tmp, data1);
	}

	for (i = 3; i < 8; ++i) {
		tmp.inode_offset = i;
		fs_create(&fs1, &tmp, fs1.sector_size * 2, 1, true);
		inodes[i] = tmp;
		fs_write(&fs1, &tmp, data2);
	}

	for (i = 0; i < 4; ++i)
		fs_delete(&fs1, &inodes[i * 2]);

	defragment(&fs1);

	buf = malloc(fs1.sector_size * fs1.alloc_table_size);

	disk_read(disk1, buf, fs1.alloc_table, fs1.alloc_table_size);

	TEST_ASSERT_EQUAL_HEX8(0xFF, buf[0]);
	TEST_ASSERT_EQUAL_HEX8(0xE0, buf[1]);

	free(buf);

	buf = malloc(fs1.sector_size * 2);
	for (i = 1; i < 4; ++i) {
		fs_open(&fs1, inodes[i * 2 + 1].inode_offset, &tmp);
		TEST_ASSERT_EQUAL(inodes[i * 2 + 1].size, tmp.size);
		fs_read(&fs1, &tmp, buf, tmp.size);

		for (k = 0; k < tmp.size; ++k)
			TEST_ASSERT_EQUAL_HEX8(data2[k], buf[k]);

	}
	free(buf);
}

TEST(tasks_tests, delete_invalid_inodes_test)
{
	uint now, tmp, free_disk_space;
	enum FSRESULT ret_val;

	now = (uint) time(NULL);

	fs_mount(disk1, &fs1, &win);

	free_disk_space = fs_getfree(&fs1);
	fs_create(&fs1, &in1, 100, now + 100, true);
	fs_create(&fs1, &in2, 100, now + 100, false);
	fs_create(&fs1, &in3, 1000, now - 100, true);
	fs_create(&fs1, &in4, 1000, now - 100, false);

	tmp = inodes_used(&fs1);
	TEST_ASSERT_EQUAL_UINT(4, tmp);
	delete_invalid_inodes(&fs1);
	tmp = inodes_used(&fs1);
	TEST_ASSERT_EQUAL_UINT(2, tmp);
	tmp = free_disk_space - (fs1.sector_size * 4);
	TEST_ASSERT_EQUAL_UINT(tmp, fs_getfree(&fs1));

	ret_val = fs_open(&fs1, in3.inode_offset, &in3);
	TEST_ASSERT_EQUAL(FS_PARAM_ERROR, ret_val);
	ret_val = fs_open(&fs1, in4.inode_offset, &in4);
	TEST_ASSERT_EQUAL(FS_PARAM_ERROR, ret_val);
}

TEST(tasks_tests, restore_fs_test)
{
	int i;
	uint8_t *tmp, *tmp_cpy;

	fs_mount(disk1, &fs1, &win);

	fs_create(&fs1, &in1, 100, 100, false);
	fs_create(&fs1, &in2, 100, 100, false);
	fs_delete(&fs1, &in1);
	fs_create(&fs1, &in3, 2000, 100, true);
	fs_create(&fs1, &in1, 1001, 100, false);
	fs_delete(&fs1, &in3);
	/* 3000 0000 03ff fe */
	tmp = malloc(fs1.alloc_table_size * fs1.sector_size);
	tmp_cpy = malloc(fs1.alloc_table_size * fs1.sector_size);
	disk_read(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);
	memcpy(tmp_cpy, tmp, fs1.alloc_table_size * fs1.sector_size);
	tmp[1] = 0xFF;
	tmp[2] = 0xFF;
	disk_write(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);
	/* 30FF FF00 03ff fe */
	restore_fs(&fs1);
	disk_read(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	for (i = 0; i  < fs1.alloc_table_size * fs1.sector_size; ++i)
		TEST_ASSERT_EQUAL_HEX8(tmp_cpy[i], tmp[i]);

	free(tmp);
	free(tmp_cpy);
}

TEST(tasks_tests, restore_fs_test2)
{
	int i;
	uint8_t *tmp, *tmp_cpy;

	fs_mount(disk1, &fs1, &win);

	fs_create(&fs1, &in1, 100, 1000, false);
	fs_create(&fs1, &in2, 500, 100, false);
	fs_delete(&fs1, &in1);
	fs_create(&fs1, &in3, 200, 100, true);
	fs_create(&fs1, &in1, 1001, 100, false);
	fs_delete(&fs1, &in3);
	/* 3fe1 ffff 0000 001f */
	tmp = malloc(fs1.alloc_table_size * fs1.sector_size);
	tmp_cpy = malloc(fs1.alloc_table_size * fs1.sector_size);
	disk_read(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);
	memcpy(tmp_cpy, tmp, fs1.alloc_table_size * fs1.sector_size);
	for (i = 0; i < fs1.alloc_table_size * fs1.sector_size; ++i)
		tmp[i] = 0xFF;
	disk_write(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	restore_fs(&fs1);
	disk_read(disk1, tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	for (i = 0; i  < fs1.alloc_table_size * fs1.sector_size; ++i)
		TEST_ASSERT_EQUAL_HEX8(tmp_cpy[i], tmp[i]);

	free(tmp);
	free(tmp_cpy);
}

TEST(tasks_tests, delete_invalid_inodes_test2)
{
	uint i, now;
	struct timeval t;
	int tmp;

	fs_mount(disk1, &fs1, &win);

	gettimeofday(&t, NULL);
	now = (uint) t.tv_sec;

	tmp = 1;
	for (i = 0; i < 10; ++i) {
		fs_create(&fs1, &in1, 100, now + tmp, true);
		tmp *= -1;
	}

	TEST_ASSERT_EQUAL_UINT(10, inodes_used(&fs1));
	delete_invalid_inodes(&fs1);
	TEST_ASSERT_EQUAL_UINT(5, inodes_used(&fs1));

	sleep(2);

	delete_invalid_inodes(&fs1);
	TEST_ASSERT_EQUAL_UINT(0, inodes_used(&fs1));

}

TEST_GROUP_RUNNER(tasks_tests)
{
	RUN_TEST_CASE(tasks_tests, defragment_test);
	RUN_TEST_CASE(tasks_tests, defragment_test2);
	RUN_TEST_CASE(tasks_tests, delete_invalid_inodes_test);
	RUN_TEST_CASE(tasks_tests, delete_invalid_inodes_test2);
	RUN_TEST_CASE(tasks_tests, restore_fs_test);
	RUN_TEST_CASE(tasks_tests, restore_fs_test2);
}
