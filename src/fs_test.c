#include "../h/fs_test.h"

struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

extern unsigned long div_up(unsigned long dividend,
	unsigned long divisor);

bool fs_mount_test(void)
{
	uint i, tmp;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 4; ++i) {
		disk_initialize(disks[i]);
		fs_mount(disks[i], &fs1);

		if (fs1.sector_size != disks[i]->sector_size)
			return false;
		if (fs1.sector_count != disks[i]->sector_count)
			return false;
		tmp = div_up(sizeof(struct FILE_SYSTEM), fs1.sector_size);
		if (fs1.alloc_table != tmp)
			return false;
		tmp = div_up(fs1.sector_count, fs1.sector_size * 8);
		if (fs1.alloc_table_size != tmp)
			return false;
		tmp += fs1.alloc_table;
		if (fs1.inode_alloc_table != tmp)
			return false;
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		tmp = div_up(tmp, fs1.sector_size);
		if (fs1.inode_alloc_table_size != tmp)
			return false;
		tmp += fs1.inode_alloc_table;
		if (fs1.inode_block != tmp)
			return false;
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		if (fs1.inode_block_size != tmp)
			return false;
	}

	return true;
}

bool mkfs_test(void)
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
		if (fs->sector_size != disks[k]->sector_size)
			return false;
		if (fs->sector_count != disks[k]->sector_count)
			return false;
		if (fs->alloc_table != 1)
			return false;
		if (fs->alloc_table_size != at_size)
			return false;
		if (fs->inode_alloc_table != (at_size + 1))
			return false;
		if (fs->inode_alloc_table_size != it_size)
			return false;
		if (fs->inode_block != (at_size + it_size + 1))
			return false;
		if (fs->inode_block_size != ib_size)
			return false;
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
	return true;
}

void fs_test_setUp(void)
{
	uint i;

	/*Struct setup*/
	disk1 = disk_fill("disks/disk1.disk", 1024, 64);
	disk2 = disk_fill("disks/disk2.disk", 2048, 128);
	disk3 = disk_fill("disks/disk3.disk", 4096, 256);
	disk4 = disk_fill("disks/disk4.disk", 65536, 512);

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

	/*Force the fs to write*/
	disk_shutdown(disk1);
	disk_shutdown(disk2);
	disk_shutdown(disk3);
	disk_shutdown(disk4);

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

void fs_test_tearDown(void)
{
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

void run_fs_tests(void)
{
	uint i, length;

	bool (*tests[2]) (void);
	tests[0] = mkfs_test;
	tests[1] = fs_mount_test;

	length = sizeof(tests) / sizeof(tests[0]);
	printf("--------Start fs_tests--------\n");
	for (i = 0; i < length; ++i) {
		fs_test_setUp();
		if (!(*tests[i])())
			printf("Error in Test %d\n", i);
		else
			printf("Test %d succeeded\n", i);
		fs_test_tearDown();
	}
}
