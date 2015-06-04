#include "fs_test.h"

struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
struct INODE in1, in2, in3, in4;
uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

extern bool free_disk_space(struct disk *disk, struct FILE_SYSTEM *fs,
	uint size);
extern void delete_invalid_inodes(struct disk *disk, struct FILE_SYSTEM *fs);
extern void load_inodes_all(struct FILE_SYSTEM *fs, struct INODE *buffer);
extern bool isNotValid(struct INODE *inode);
extern bool find_first_IN_length(struct disk *disk, struct FILE_SYSTEM *fs,
	struct INODE *file, uint size);

TEST_GROUP(fs_tests);

TEST_SETUP(fs_tests)
{
	uint i;

	disk1 = malloc(sizeof(struct disk));
	disk2 = malloc(sizeof(struct disk));
	disk3 = malloc(sizeof(struct disk));
	disk4 = malloc(sizeof(struct disk));
	/*Struct setup*/
	disk_fill(disk1, "disks/disk1.disk", 1024, 64);
	disk_fill(disk2, "disks/disk2.disk", 2048, 128);
	disk_fill(disk3, "disks/disk3.disk", 4096, 256);
	disk_fill(disk4, "disks/disk4.disk", 65536, 512);

	/*File setup*/
	disk_create(disk1, 1024);
	disk_create(disk2, 2048);
	disk_create(disk3, 4096);
	disk_create(disk4, 65536);

	/*Opens the file*/
	disk_initialize(disk1);
	disk_initialize(disk2);
	disk_initialize(disk3);
	disk_initialize(disk4);

	/*Make disk structures*/
	fs_mkfs(disk1);
	fs_mkfs(disk2);
	fs_mkfs(disk3);
	fs_mkfs(disk4);

	/*Force the disk to write*/
	disk_shutdown(disk1);
	disk_shutdown(disk2);
	disk_shutdown(disk3);
	disk_shutdown(disk4);

	disk_initialize(disk1);
	disk_initialize(disk2);
	disk_initialize(disk3);
	disk_initialize(disk4);

	/* Inode filling */
	in1.id = 0;
	in1.size = 10;
	in1.check_size = 1;
	in1.creation_date = 0x0F;
	in1.last_modified = 0x0F;
	in1.location = 0;
	in1.inode_offset = 0;
	in1.custody = false;
	in1.time_to_live = 100;

	in2.id = 3;
	in2.size = 1;
	in2.check_size = 1;
	in2.creation_date = 0xFF00;
	in2.last_modified = 0xFFFF;
	in2.location = 10;
	in2.inode_offset = 1;
	in2.custody = true;
	in2.time_to_live = 100;

	in3.id = 10;
	in3.size = 2;
	in3.check_size = 1;
	in3.creation_date = 0xFF00;
	in3.last_modified = 0xFFFF;
	in3.location = 11;
	in3.inode_offset = 2;
	in3.custody = true;
	in3.time_to_live = 100;

	in4.id = 10;
	in4.size = 2;
	in4.check_size = 1;
	in4.creation_date = 0xFF00;
	in4.last_modified = 0xFFFF;
	in4.location = 11;
	in4.inode_offset = 2;
	in4.custody = true;
	in4.time_to_live = (uint) time(NULL) + 500;

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

TEST_TEAR_DOWN(fs_tests)
{
	disk_shutdown(disk1);
	disk_shutdown(disk2);
	disk_shutdown(disk3);
	disk_shutdown(disk4);
	free(disk1);
	free(disk2);
	free(disk3);
	free(disk4);
	free(table1);
	free(table2);
	free(table1_empty);
	free(table2_empty);
	free(data1);
	free(data2);
	free(data3);
}

TEST(fs_tests, free_disk_space_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}

TEST(fs_tests, fs_create_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}

TEST(fs_tests, delete_invalid_inodes_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}

TEST(fs_tests, find_first_IN_length_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}


TEST(fs_tests, fs_write_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}

TEST(fs_tests, fs_read_test)
{
	TEST_ASSERT_EQUAL(1, 0);
}

/* TODO: Check if the alloc tables where changed*/
TEST(fs_tests, fs_delete_test)
{
	uint8_t *buffer, offset, i;
	struct INODE *tmp;

	tmp = malloc(sizeof(struct INODE));

	fs_mount(disk1, &fs1);
	fs_close(&fs1, &in1);
	fs_close(&fs1, &in2);
	fs_close(&fs1, &in3);

	fs_delete(&fs1, &in1);
	TEST_ASSERT_EQUAL(fs_open(&fs1, in1.id, tmp), FS_ERROR);
	TEST_ASSERT_EQUAL(fs_open(&fs1, in2.id, tmp), FS_OK);
	fs_delete(&fs1, &in2);
	TEST_ASSERT_EQUAL(fs_open(&fs1, in2.id, tmp), FS_ERROR);

	offset = fs1.inode_block + in3.inode_offset;
	fs_delete(&fs1, &in3);
	buffer = malloc(fs1.sector_size);

	disk_read(fs1.disk, (char *) buffer, offset, 1);

	for (i = 0; i < fs1.sector_size; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], 0X00);

	/*TODO: Write into the alloc table first */
	disk_read(fs1.disk, (char *) buffer, fs1.inode_alloc_table, 1);
	buffer[0] &= 0x80 >> 2;
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0x00);

	free(tmp);
	free(buffer);
}

TEST(fs_tests, fs_getfree_test)
{
	uint i;
	struct FILE_SYSTEM fs[3] = {fs1, fs2, fs3};
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 3; ++i) {
		fs_mount(disks[i], &fs[i]);
		TEST_ASSERT_EQUAL_UINT(fs_getfree(&fs[i]),
			fs[i].sector_count * fs[i].sector_size);
	}
}

TEST(fs_tests, isNotValid_test)
{
	uint i;
	struct INODE inodes[3] = {in1, in2, in3};

	for (i = 0; i < 3; ++i)
		TEST_ASSERT_TRUE(isNotValid(&inodes[i]));
	TEST_ASSERT_FALSE(isNotValid(&in4));
}

TEST(fs_tests, load_inodes_all_test)
{
	uint i;
	struct INODE *tmp;
	struct INODE inodes[3] = {in1, in2, in3};

	tmp = malloc(sizeof(struct INODE));

	fs_mount(disk1, &fs1);
	fs_close(&fs1, &in1);
	fs_close(&fs1, &in2);
	fs_close(&fs1, &in3);

	free(tmp);
	tmp = malloc(fs1.inode_block_size * sizeof(struct INODE));
	load_inodes_all(&fs1, tmp);

	for (i = 0; i < 3; ++i)
		TEST_ASSERT_EQUAL_UINT(tmp[i].id, inodes[i].id);
	free(tmp);
}

TEST(fs_tests, fs_close_test)
{
	uint8_t *buffer, i;
	struct INODE *tmp;
	struct INODE inodes[3] = {in1, in2, in3};

	tmp = malloc(sizeof(struct INODE));

	fs_mount(disk1, &fs1);
	fs_close(&fs1, &in1);
	fs_close(&fs1, &in2);
	fs_close(&fs1, &in3);

	buffer = malloc(fs1.inode_block_size * fs1.sector_size);
	disk_read(disk1, (char *) buffer, fs1.inode_block,
		fs1.inode_block_size);

	for (i = 0; i < 3; ++i) {
		memcpy(tmp, buffer, sizeof(struct INODE));
		TEST_ASSERT_EQUAL_UINT(tmp->size, inodes[i].size);
		TEST_ASSERT_EQUAL_UINT(tmp->id, inodes[i].id);
		TEST_ASSERT_EQUAL_UINT(tmp->check_size, inodes[i].check_size);
		TEST_ASSERT_EQUAL_UINT(tmp->creation_date,
			inodes[i].creation_date);
		TEST_ASSERT_EQUAL_UINT(tmp->last_modified,
			inodes[i].last_modified);
		TEST_ASSERT_EQUAL_UINT(tmp->location, inodes[i].location);
		TEST_ASSERT_EQUAL_UINT(tmp->inode_offset,
			inodes[i].inode_offset);
		TEST_ASSERT_EQUAL(tmp->custody, inodes[i].custody);
		TEST_ASSERT_EQUAL_UINT(tmp->time_to_live,
			inodes[i].time_to_live);
		buffer += fs1.sector_size;
	}
	free(tmp);
	buffer -= fs1.sector_size * 3;
	free(buffer);
}

TEST(fs_tests, fs_open_test)
{
	uint8_t *buffer, i;
	struct INODE *tmp;
	struct INODE inodes[3] = {in1, in2, in3};

	fs_mount(disk2, &fs1);
	buffer = malloc(fs1.sector_size);
	tmp = malloc(sizeof(struct INODE));

	for (i = 0; i < 3; ++i) {
		memcpy(buffer, &inodes[i], sizeof(struct INODE));
		disk_write(disk2, (char *) buffer, fs1.inode_block + i, 1);

		fs_open(&fs1, inodes[i].id, tmp);

		TEST_ASSERT_EQUAL_UINT(tmp->size, inodes[i].size);
		TEST_ASSERT_EQUAL_UINT(tmp->id, inodes[i].id);
		TEST_ASSERT_EQUAL_UINT(tmp->check_size, inodes[i].check_size);
		TEST_ASSERT_EQUAL_UINT(tmp->creation_date,
			inodes[i].creation_date);
		TEST_ASSERT_EQUAL_UINT(tmp->last_modified,
			inodes[i].last_modified);
		TEST_ASSERT_EQUAL_UINT(tmp->location, inodes[i].location);
		TEST_ASSERT_EQUAL_UINT(tmp->inode_offset,
			inodes[i].inode_offset);
		TEST_ASSERT_EQUAL(tmp->custody, inodes[i].custody);
		TEST_ASSERT_EQUAL_UINT(tmp->time_to_live,
			inodes[i].time_to_live);
	}

	free(tmp);
	free(buffer);
}

TEST(fs_tests, fs_mount_test)
{
	uint i, tmp;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 4; ++i) {
		disk_initialize(disks[i]);
		fs_mount(disks[i], &fs1);

		TEST_ASSERT_EQUAL_UINT(fs1.sector_size, disks[i]->sector_size);
		TEST_ASSERT_EQUAL_UINT(fs1.sector_count,
			disks[i]->sector_count);
		tmp = div_up(sizeof(struct FILE_SYSTEM), fs1.sector_size);
		TEST_ASSERT_EQUAL_UINT(fs1.alloc_table, tmp);
		tmp = div_up(fs1.sector_count, fs1.sector_size * 8);
		TEST_ASSERT_EQUAL_UINT(fs1.alloc_table_size, tmp);
		tmp += fs1.alloc_table;
		TEST_ASSERT_EQUAL_UINT(fs1.inode_alloc_table, tmp);
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		tmp = div_up(tmp, fs1.sector_size);
		TEST_ASSERT_EQUAL_UINT(fs1.inode_alloc_table_size, tmp);
		tmp += fs1.inode_alloc_table;
		TEST_ASSERT_EQUAL_UINT(fs1.inode_block, tmp);
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		TEST_ASSERT_EQUAL_UINT(fs1.inode_block_size, tmp);
	}
}

TEST(fs_tests, fs_mkfs_test)
{
	uint k, at_size, it_size, ib_size;
	uint8_t *buffer;
	struct FILE_SYSTEM *fs;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (k = 0; k < 4; ++k) {

		buffer = malloc(sizeof(uint8_t) * disks[k]->sector_size);
		disk_initialize(disks[k]);
		fs_mkfs(disks[k]);
		at_size = div_up(disks[k]->sector_count,
			disks[k]->sector_size);
		ib_size = disks[k]->sector_count / 8;
		if (ib_size < 8)
			ib_size = 8;
		it_size = div_up(ib_size, disks[k]->sector_size);

		disk_read(disks[k], (char *) buffer, 0, 1);

		fs = (struct FILE_SYSTEM *) buffer;
		TEST_ASSERT_EQUAL_UINT(fs->sector_size, disks[k]->sector_size);
		TEST_ASSERT_EQUAL_UINT(fs->sector_count,
			disks[k]->sector_count);
		TEST_ASSERT_EQUAL_UINT(fs->alloc_table, 1);
		TEST_ASSERT_EQUAL_UINT(fs->alloc_table_size, at_size);
		TEST_ASSERT_EQUAL_UINT(fs->inode_alloc_table, (at_size + 1));
		TEST_ASSERT_EQUAL_UINT(fs->inode_alloc_table_size, it_size);
		TEST_ASSERT_EQUAL_UINT(fs->inode_block,
			(at_size + it_size + 1));
		TEST_ASSERT_EQUAL_UINT(fs->inode_block_size, ib_size);
		fs = NULL;

		/* TODO:
		disk_read(disks[k], (char *) buffer, 1, at_size);
		for (i = 0; i < (disks[k]->sector_count/8); ++i)
			if (buffer[i] != 0x00)
				return false;
		for (i = 2; i < disks[k]->sector_size; ++i)
			if (buffer[i] != 0xFF)
				return false; */

		disk_shutdown(disks[k]);
		free(buffer);
	}
}

TEST_GROUP_RUNNER(fs_tests)
{
	RUN_TEST_CASE(fs_tests, fs_mkfs_test);
	RUN_TEST_CASE(fs_tests, fs_mount_test);
	RUN_TEST_CASE(fs_tests, free_disk_space_test);
	RUN_TEST_CASE(fs_tests, isNotValid_test);
	RUN_TEST_CASE(fs_tests, load_inodes_all_test);
	RUN_TEST_CASE(fs_tests, delete_invalid_inodes_test);
	RUN_TEST_CASE(fs_tests, find_first_IN_length_test);
	RUN_TEST_CASE(fs_tests, fs_getfree_test);
	RUN_TEST_CASE(fs_tests, fs_write_test);
	RUN_TEST_CASE(fs_tests, fs_read_test);
	RUN_TEST_CASE(fs_tests, fs_delete_test);
	RUN_TEST_CASE(fs_tests, fs_close_test);
	RUN_TEST_CASE(fs_tests, fs_open_test);
	RUN_TEST_CASE(fs_tests, fs_create_test);
}
