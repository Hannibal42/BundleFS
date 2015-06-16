#include "fs_test.h"

struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
struct INODE in1, in2, in3, in4;
uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

extern bool free_disk_space(struct FILE_SYSTEM *fs,
	uint size);
extern void delete_invalid_inodes(struct FILE_SYSTEM *fs);
extern bool isNotValid(struct INODE *inode);
extern bool find_first_IN_length(struct FILE_SYSTEM *fs, struct INODE *file,
	uint size);
extern void write_inode(struct FILE_SYSTEM *fs, struct INODE *file);
extern uint inodes_used(struct FILE_SYSTEM *fs);
extern void load_inodes_all_full(struct FILE_SYSTEM *fs, struct INODE *buffer);
extern void defragment(struct FILE_SYSTEM *fs);
extern bool resize_inode_block(struct FILE_SYSTEM *fs);

TEST_GROUP(fs_tests);

TEST_SETUP(fs_tests)
{
	uint i;

	disk1 = malloc(sizeof(struct disk));
	disk2 = malloc(sizeof(struct disk));
	disk3 = malloc(sizeof(struct disk));
	disk4 = malloc(sizeof(struct disk));
	/*Struct setup*/
	disk_fill(disk1, "disks/disk1.disk", 4096, 64);
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
	int i;
	uint now;
	struct INODE *tmp;

	tmp = malloc(sizeof(struct INODE));
	now = (uint) time(NULL);

	fs_mount(disk1, &fs1);

	fs_create(&fs1, tmp, 1, 0, false);

	TEST_ASSERT_FALSE(free_disk_space(&fs1, 100));
	TEST_ASSERT_TRUE(free_disk_space(&fs1, 1));
	TEST_ASSERT_FALSE(free_disk_space(&fs1, 1));

	for (i = 0; i < 2; ++i)
		fs_create(&fs1, tmp, 100, 0, false);

	TEST_ASSERT_TRUE(free_disk_space(&fs1, 100));
	TEST_ASSERT_TRUE(free_disk_space(&fs1, 100));

	fs_create(&fs1, tmp, 1000, 0, true);
	TEST_ASSERT_TRUE(free_disk_space(&fs1, 1000));
	fs_create(&fs1, tmp, 1000, now + 10000, true);
	TEST_ASSERT_FALSE(free_disk_space(&fs1, 1000));
	fs_create(&fs1, tmp, 1000, now + 10000, false);
	TEST_ASSERT_TRUE(free_disk_space(&fs1, 1000));

	free(tmp);
}

/* TODO:More test cases for the inode block resizing */
TEST(fs_tests, fs_create_test)
{
	uint i, tmp_time;
	int tmp;
	struct INODE inodes[3] = {in1, in2, in3};
	uint8_t *al_tab, *in_tab, tmp_byte;
	enum FSRESULT res;

	al_tab = malloc(fs1.sector_size * fs1.alloc_table_size);
	in_tab = malloc(fs1.sector_size * fs1.inode_alloc_table_size);

	fs_mount(disk1, &fs1);
	tmp = fs_getfree(&fs1) / fs1.sector_size;
	for (i = 0; i < 8; ++i) {
		tmp_time = (uint) time(NULL) + 10000;
		res = fs_create(&fs1, &inodes[0], 8 * fs1.sector_size,
			tmp_time, true);
		tmp -= 8;
		if (tmp > 8) {
			disk_read(disk1, (char *) al_tab, fs1.alloc_table,
				fs1.alloc_table_size);
			TEST_ASSERT_EQUAL_HEX8(al_tab[i], 0xFF);
			disk_read(disk1, (char *) in_tab, fs1.inode_alloc_table,
				fs1.inode_alloc_table_size);
			tmp_byte = 0x80 >> i;
			TEST_ASSERT_EQUAL(in_tab[0] & tmp_byte, tmp_byte);
		} else {
			TEST_ASSERT_EQUAL(res, FS_FULL);
		}
	}

	fs_mount(disk2, &fs2);
	tmp = fs_getfree(&fs2) - fs2.sector_size - 200;
	res = fs_create(&fs2, &inodes[0], tmp, 100, false);
	TEST_ASSERT_EQUAL(FS_OK, res);
	res = fs_create(&fs2, &inodes[0], tmp, 100, true);
	TEST_ASSERT_EQUAL(FS_OK, res);

	free(al_tab);
	free(in_tab);
}

TEST(fs_tests, delete_invalid_inodes_test)
{
	uint now, tmp, free_disk_space;
	enum FSRESULT ret_val;

	now = (uint) time(NULL);

	fs_mount(disk1, &fs1);

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
	tmp = free_disk_space - (fs1.sector_size * 6);
	TEST_ASSERT_EQUAL_UINT(tmp, fs_getfree(&fs1));

	ret_val = fs_open(&fs1, in3.id, &in3);
	TEST_ASSERT_EQUAL(FS_ERROR, ret_val);
	ret_val = fs_open(&fs1, in4.id, &in4);
	TEST_ASSERT_EQUAL(FS_ERROR, ret_val);
}

TEST(fs_tests, find_first_IN_length_test)
{
	struct INODE tmp;

	fs_mount(disk1, &fs1);

	fs_create(&fs1, &in1, 128, 100, false);
	TEST_ASSERT_TRUE(find_first_IN_length(&fs1, &tmp, 128));
	fs_create(&fs1, &in1, 128, 100, true);
	fs_create(&fs1, &in1, 64, 100, false);
	fs_create(&fs1, &in1, 256, 100, false);
	TEST_ASSERT_TRUE(find_first_IN_length(&fs1, &tmp, 256));
	TEST_ASSERT_EQUAL(in1.id, tmp.id);
}


TEST(fs_tests, fs_write_test)
{
	int i;
	uint8_t *buffer, *buffer2;
	struct INODE *tmp;
	enum FSRESULT ret_val;

	tmp = malloc(sizeof(struct INODE));
	buffer = malloc(100);
	buffer2 = malloc(128);

	fs_mount(disk1, &fs1);
	fs_create(&fs1, tmp, 100, 10, true);

	for (i = 0; i < 100; ++i)
		buffer[i] = 0xFA;

	ret_val = fs_write(&fs1, tmp, (char *) buffer);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	disk_read(disk1, (char *) buffer2, tmp->location, 2);

	for (i = 0; i < 100; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], buffer2[i]);

	free(buffer2);
	free(buffer);

	fs_create(&fs1, tmp, 1024, 100, false);
	buffer = malloc(1024);
	buffer2 = malloc(1024);

	for (i = 0; i < 1024; ++i) {
		if (i < 530)
			buffer[i] = 0xEA;
		else
			buffer[i] = 0x03;
	}
	ret_val = fs_write(&fs1, tmp, (char *) buffer);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	disk_read(disk1, (char *) buffer2, tmp->location, 16);

	for (i = 0; i < 100; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], buffer2[i]);

	free(buffer2);
	free(buffer);
	free(tmp);
}

TEST(fs_tests, fs_read_test)
{
	int i;
	uint8_t *buffer, *buffer2;
	struct INODE *tmp;
	enum FSRESULT ret_val;

	tmp = malloc(sizeof(struct INODE));
	buffer = malloc(100);
	buffer2 = malloc(100);

	fs_mount(disk1, &fs1);
	fs_create(&fs1, tmp, 100, 10, true);

	for (i = 0; i < 100; ++i)
		buffer[i] = 0xFA;
	ret_val = fs_write(&fs1, tmp, (char *) buffer);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	ret_val = fs_read(&fs1, tmp, (char *) buffer2, tmp->size);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	for (i = 0; i < 100; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], buffer2[i]);
	free(buffer2);
	free(buffer);

	fs_create(&fs1, tmp, 1000, 10, false);
	buffer = malloc(1000);
	buffer2 = malloc(1000);

	for (i = 0; i < 1000; ++i)
		buffer[i] = 0xF0;
	ret_val = fs_write(&fs1, tmp, (char *) buffer);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	ret_val = fs_read(&fs1, tmp, (char *) buffer2, tmp->size);
	TEST_ASSERT_EQUAL(FS_OK, ret_val);
	for (i = 0; i < 1000; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], buffer2[i]);
	ret_val = fs_read(&fs1, tmp, (char *) buffer2, 1001);

	TEST_ASSERT_EQUAL(FS_PARAM_ERROR, ret_val);
	buffer[0] = 0xFF;
	disk_write(disk1, (char *) buffer, tmp->location + 16, 1);
	ret_val = fs_read(&fs1, tmp, (char *) buffer2, tmp->size);
	TEST_ASSERT_EQUAL(FS_CHECK_ERROR, ret_val);

	free(buffer2);
	free(buffer);
	free(tmp);
}

TEST(fs_tests, fs_delete_test2)
{
	uint8_t *buffer;

	buffer = malloc(fs1.sector_size);
	fs_mount(disk1, &fs1);
	fs_create(&fs1, &in1, 1, 100000, true);
	fs_create(&fs1, &in2, 1, 100000, true);
	fs_create(&fs1, &in3, 1, 100000, true);

	disk_read(disk1, (char *) buffer, fs1.alloc_table, 1);
	fs_delete(&fs1, &in2);

	free(buffer);
	buffer = malloc(fs1.sector_size);

	disk_read(disk1, (char *) buffer, fs1.alloc_table, 1);
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0xA0);

	disk_read(disk1, (char *) buffer, fs1.inode_alloc_table, 1);
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0xA0);

	fs_delete(&fs1, &in3);

	disk_read(disk1, (char *) buffer, fs1.alloc_table, 1);
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0x80);

	disk_read(disk1, (char *) buffer, fs1.inode_alloc_table, 1);
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0x80);

	free(buffer);
}

TEST(fs_tests, fs_delete_test)
{
	uint8_t *buffer, offset, i;
	struct INODE *tmp;

	tmp = malloc(sizeof(struct INODE));

	fs_mount(disk1, &fs1);
	in1.id = 42;
	in2.id = 2000;
	fs_create(&fs1, &in1, 100, 100, false);
	fs_create(&fs1, &in2, 100, 100, false);
	fs_create(&fs1, &in3, 100, 100, false);

	TEST_ASSERT_EQUAL(FS_OK, fs_delete(&fs1, &in1));
	TEST_ASSERT_EQUAL(fs_open(&fs1, in2.id, tmp), FS_OK);
	TEST_ASSERT_EQUAL(fs_open(&fs1, 42, tmp), FS_ERROR);
	TEST_ASSERT_EQUAL(FS_OK, fs_delete(&fs1, &in2));
	TEST_ASSERT_EQUAL(fs_open(&fs1, 2000, tmp), FS_ERROR);

	offset = fs1.inode_block + in3.inode_offset;
	fs_delete(&fs1, &in3);
	buffer = malloc(fs1.sector_size);

	disk_read(fs1.disk, (char *) buffer, offset, 1);

	for (i = 0; i < fs1.sector_size; ++i)
		TEST_ASSERT_EQUAL_HEX8(buffer[i], 0X00);

	disk_read(fs1.disk, (char *) buffer, fs1.alloc_table, 1);

	disk_read(fs1.disk, (char *) buffer, fs1.inode_alloc_table, 1);
	buffer[0] &= 0x80 >> 2;
	TEST_ASSERT_EQUAL_HEX8(buffer[0], 0x00);
	free(tmp);
	free(buffer);
}

TEST(fs_tests, fs_getfree_test)
{
	uint i, tmp;
	struct FILE_SYSTEM fs[3] = {fs1, fs2, fs3};
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 3; ++i) {
		fs_mount(disks[i], &fs[i]);
		tmp = fs[i].sector_count;
		tmp -= fs[i].inode_block + fs[i].inode_block_size;
		TEST_ASSERT_EQUAL_UINT(fs_getfree(&fs[i]),
			tmp * fs[i].sector_size);
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
		fs_create(&fs1, &inodes[i], 100, 100, true);

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

	TEST_ASSERT_EQUAL(FS_ERROR, fs_open(&fs1, 10101, &in1));

	free(tmp);
	free(buffer);
}

TEST(fs_tests, fs_mount_test)
{
	uint i, tmp;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 4; ++i) {
		disk_initialize(disks[i]);
		TEST_ASSERT_EQUAL(FS_OK, fs_mount(disks[i], &fs1));

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
		TEST_ASSERT_EQUAL_UINT(fs1.inode_max, tmp);
		TEST_ASSERT_EQUAL_UINT(fs1.sector_size / sizeof(struct INODE),
			fs1.inode_sec);
		TEST_ASSERT_EQUAL_UINT(div_up(fs1.inode_max, fs1.inode_sec),
			fs1.inode_block_size);
	}
}

TEST(fs_tests, fs_mkfs_test)
{
	int i;
	uint k, at_size, it_size, ino_max, ib_size, tmp;
	uint8_t *buffer;
	struct FILE_SYSTEM *fs;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (k = 0; k < 4; ++k) {

		buffer = malloc(disks[k]->sector_size);
		disk_initialize(disks[k]);
		TEST_ASSERT_EQUAL(FS_OK, fs_mkfs(disks[k]));
		at_size = div_up(disks[k]->sector_count,
			disks[k]->sector_size);
		ino_max = disks[k]->sector_count / 8;
		if (ino_max < 8)
			ino_max = 8;
		it_size = div_up(ino_max, disks[k]->sector_size);

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
		TEST_ASSERT_EQUAL_UINT(fs->inode_max, ino_max);
		TEST_ASSERT_EQUAL_UINT(fs->sector_size / sizeof(struct INODE),
			fs->inode_sec);
		TEST_ASSERT_EQUAL_UINT(div_up(ino_max, fs->inode_sec),
			fs->inode_block_size);
		ib_size = fs->inode_block_size;

		disk_read(disks[k], (char *) buffer, 1, at_size);
		tmp = disks[k]->sector_count;
		tmp -= (1 + at_size + it_size + ib_size);
		tmp /= 8;
		for (i = 0; i < tmp; ++i)
			TEST_ASSERT_EQUAL(buffer[i], 0x00);

		disk_read(disks[k], (char *) buffer, 2, it_size);
		tmp = ino_max / 8;
		for (i = 0; i < tmp; ++i)
			TEST_ASSERT_EQUAL(buffer[i], 0x00);

		fs = NULL;
		disk_shutdown(disks[k]);
		free(buffer);
	}
}

TEST(fs_tests, write_inode_test)
{
	uint i;
	uint8_t *tmp;
	struct INODE inodes[3] = {in1, in2, in3};
	struct INODE *tmp_in;


	fs_mount(disk1, &fs1);

	for (i = 0; i < 3; ++i) {
		write_inode(&fs1, &inodes[i]);
		tmp = malloc(fs1.sector_size);
		disk_read(disk1, (char *) tmp, fs1.inode_block +
			inodes[i].inode_offset, 1);
		tmp_in = (struct INODE *) tmp;

		TEST_ASSERT_EQUAL(tmp_in->location, inodes[i].location);
		free(tmp);
		tmp_in = NULL;
	}
}

TEST(fs_tests, inodes_used_test)
{
	uint i, k, tmp;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};
	struct INODE inodes[3] = {in1, in2, in3};

	for (k = 0; k < 4; ++k) {
		tmp = 0;
		fs_mount(disks[k], &fs1);
		for (i = 0; i < 2; ++i) {
			TEST_ASSERT_EQUAL_UINT(inodes_used(&fs1), tmp);
			if (fs_create(&fs1, &inodes[i], 64, 1, true) == FS_OK)
				++tmp;
		}
		TEST_ASSERT_EQUAL_UINT(inodes_used(&fs1), tmp);
	}
}

TEST(fs_tests, load_inodes_all_full_test)
{
	uint i;
	struct INODE *tmp;
	struct INODE inodes[3] = {in1, in2, in3};

	fs_mount(disk1, &fs1);

	for (i = 0; i < 3; ++i)
		fs_create(&fs1, &inodes[i], 1, 1, true);

	tmp = malloc(sizeof(struct INODE) * inodes_used(&fs1));
	load_inodes_all_full(&fs1, tmp);

	for (i = 0; i < 3; ++i)
		TEST_ASSERT_EQUAL_UINT(tmp[i].id, inodes[i].id);

	free(tmp);
	fs_delete(&fs1, &inodes[1]);

	tmp = malloc(sizeof(struct INODE) * inodes_used(&fs1));
	load_inodes_all_full(&fs1, tmp);

	TEST_ASSERT_EQUAL_UINT(tmp[0].id, inodes[0].id);
	TEST_ASSERT_EQUAL_UINT(tmp[1].id, inodes[2].id);

	free(tmp);
}

TEST(fs_tests, defragment_test)
{
	uint i, k;
	struct INODE inodes[3] = {in1, in2, in3};
	struct INODE tmp;

	fs_mount(disk1, &fs1);

	for (i = 0; i < 3; ++i)
		fs_create(&fs1, &inodes[i], fs1.sector_size, 1, true);

	fs_delete(&fs1, &inodes[1]);

	defragment(&fs1);

	fs_open(&fs1, inodes[0].id, &tmp);

	TEST_ASSERT_EQUAL(tmp.location, 62);
	TEST_ASSERT_EQUAL(tmp.inode_offset, 0);
	fs_delete(&fs1, &tmp);

	fs_open(&fs1, inodes[2].id, &tmp);
	TEST_ASSERT_EQUAL(tmp.location, 60);
	TEST_ASSERT_EQUAL(tmp.inode_offset, 2);
	fs_delete(&fs1, &tmp);

	for (i = 0; i < 8; ++i) {
		tmp.id = i;
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

TEST(fs_tests, defragment_test2)
{
	uint i, k;
	struct INODE tmp;
	struct INODE inodes[8];
	uint8_t *buf;

	fs_mount(disk1, &fs1);

	for (i = 0; i < 3; ++i) {
		fs_create(&fs1, &tmp, fs1.sector_size, 1, true);
		inodes[i] = tmp;
		fs_write(&fs1, &tmp, (char *) data1);
	}

	for (i = 3; i < 8; ++i) {
		tmp.id = i;
		fs_create(&fs1, &tmp, fs1.sector_size * 2, 1, true);
		inodes[i] = tmp;
		fs_write(&fs1, &tmp, (char *) data2);
	}

	for (i = 0; i < 4; ++i)
		fs_delete(&fs1, &inodes[i * 2]);

	defragment(&fs1);

	buf = malloc(fs1.sector_size * fs1.alloc_table_size);

	disk_read(disk1, (char *) buf, fs1.alloc_table, fs1.alloc_table_size);

	TEST_ASSERT_EQUAL_HEX8(0xFF, buf[0]);
	TEST_ASSERT_EQUAL_HEX8(0xE0, buf[1]);

	free(buf);

	buf = malloc(fs1.sector_size * 2);
	for (i = 1; i < 4; ++i) {
		fs_open(&fs1, inodes[i * 2 + 1].id, &tmp);
		TEST_ASSERT_EQUAL(inodes[i * 2 + 1].size, tmp.size);
		fs_read(&fs1, &tmp, (char *) buf, tmp.size);

		for (k = 0; k < tmp.size; ++k)
			TEST_ASSERT_EQUAL_HEX8(data2[k], buf[k]);

	}
	free(buf);

}

TEST(fs_tests, resize_inode_block_test)
{
	uint8_t *tmp;

	fs_mount(disk1, &fs1);
	TEST_ASSERT_TRUE(resize_inode_block(&fs1));

	tmp = malloc(fs1.sector_size * fs1.inode_alloc_table_size);
	disk_read(disk1, (char *) tmp, fs1.inode_alloc_table,
		fs1.inode_alloc_table_size);
	TEST_ASSERT_EQUAL_HEX8(0x00, tmp[0]);
	TEST_ASSERT_EQUAL_HEX8(0x00, tmp[1]);
	free(tmp);

	tmp = malloc(fs1.sector_size * fs1.alloc_table_size);
	disk_read(disk1, (char *) tmp, fs1.alloc_table,
		fs1.alloc_table_size);
	TEST_ASSERT_EQUAL_HEX8(0xFF, tmp[6]);
	TEST_ASSERT_EQUAL_HEX8(0x07, tmp[5]);
	TEST_ASSERT_EQUAL_HEX8(0x00, tmp[4]);

	TEST_ASSERT_EQUAL_UINT(16, fs1.inode_block_size);

	tmp[5] = 0x87;
	disk_write(disk1, (char *) tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	TEST_ASSERT_FALSE(resize_inode_block(&fs1));

	tmp[5] = 0x07;
	tmp[4] = 0xF8;
	disk_write(disk1, (char *) tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	TEST_ASSERT_TRUE(resize_inode_block(&fs1));

	tmp[5] = 0x07;
	tmp[4] = 0xF3;
	disk_write(disk1, (char *) tmp, fs1.alloc_table,
		fs1.alloc_table_size);

	TEST_ASSERT_FALSE(resize_inode_block(&fs1));
	free(tmp);
}

TEST_GROUP_RUNNER(fs_tests)
{
	RUN_TEST_CASE(fs_tests, fs_mkfs_test);
	RUN_TEST_CASE(fs_tests, fs_mount_test);
	RUN_TEST_CASE(fs_tests, free_disk_space_test);
	RUN_TEST_CASE(fs_tests, isNotValid_test);
	RUN_TEST_CASE(fs_tests, delete_invalid_inodes_test);
	RUN_TEST_CASE(fs_tests, find_first_IN_length_test);
	RUN_TEST_CASE(fs_tests, fs_getfree_test);
	RUN_TEST_CASE(fs_tests, fs_write_test);
	RUN_TEST_CASE(fs_tests, fs_read_test);
	RUN_TEST_CASE(fs_tests, fs_delete_test);
	RUN_TEST_CASE(fs_tests, fs_delete_test2);
	RUN_TEST_CASE(fs_tests, fs_close_test);
	RUN_TEST_CASE(fs_tests, fs_open_test);
	RUN_TEST_CASE(fs_tests, fs_create_test);
	RUN_TEST_CASE(fs_tests, write_inode_test);
	RUN_TEST_CASE(fs_tests, inodes_used_test);
	RUN_TEST_CASE(fs_tests, load_inodes_all_full_test);
	RUN_TEST_CASE(fs_tests, defragment_test);
	RUN_TEST_CASE(fs_tests, defragment_test2);
	RUN_TEST_CASE(fs_tests, resize_inode_block_test);
}
