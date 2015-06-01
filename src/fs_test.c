#include "../h/fs_test.h"
#include "../h/print_stuff.h"

struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
struct INODE in1, in2, in3, in4;
uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

extern bool free_disk_space(struct disk *disk, struct FILE_SYSTEM *fs,
	uint size);
extern void delete_invalid_inodes(struct disk *disk, struct FILE_SYSTEM *fs);
extern struct INODE *load_inodes_all(struct FILE_SYSTEM *fs);
extern bool isNotValid(struct INODE *inode);
extern bool find_first_IN_length(struct disk *disk, struct FILE_SYSTEM *fs,
	struct INODE *file, uint size);

bool free_disk_space_test(void)
{
	return false;
}

bool fs_create_test(void)
{
	return false;
}

bool delete_invalid_inodes_test(void)
{
	return false;
}

bool find_first_IN_length_test(void)
{
	return false;
}


bool fs_write_test(void)
{
	return false;
}

bool fs_read_test(void)
{
	return false;
}

/* TODO: Check if the alloc tables where changed*/
bool fs_delete_test(void)
{
	uint8_t *buffer, offset, i;
	struct INODE *tmp;

	tmp = malloc(sizeof(struct INODE));

	fs_mount(disk1, &fs1);
	fs_close(&fs1, &in1);
	fs_close(&fs1, &in2);
	fs_close(&fs1, &in3);

	fs_delete(&fs1, &in1);
	if (fs_open(&fs1, in1.id, tmp) != FS_ERROR)
		return false;
	if (fs_open(&fs1, in2.id, tmp) != FS_OK)
		return false;
	fs_delete(&fs1, &in2);
	if (fs_open(&fs1, in2.id, tmp) != FS_ERROR)
		return false;

	offset = fs1.inode_block + in3.inode_offset;
	fs_delete(&fs1, &in3);
	buffer = malloc(fs1.sector_size);

	disk_read(fs1.disk, (char *) buffer, offset, 1);

	for (i = 0; i < fs1.sector_size; ++i) {
		if (buffer[i] != 0X00)
			return false;
	}

	/*TODO: Write into the alloc table first */
	disk_read(fs1.disk, (char *) buffer, fs1.inode_alloc_table, 1);
	buffer[0] &= 0x80 >> 2;
	if (buffer[0] != 0x00)
		return false;

	free(buffer);
	return true;
}

bool fs_getfree_test(void)
{
	uint i;
	struct FILE_SYSTEM fs[3] = {fs1, fs2, fs3};
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 3; ++i) {
		fs_mount(disks[i], &fs[i]);
		if (fs_getfree(&fs[i]) != fs[i].sector_count *
			fs[i].sector_size)
			return false;
	}

	return true;
}

bool isNotValid_test(void)
{
	uint i;
	struct INODE inodes[3] = {in1, in2, in3};

	for (i = 0; i < 3; ++i)
		if (!isNotValid(&inodes[i]))
			return false;
	if (isNotValid(&in4))
		return false;

	return true;
}

bool load_inodes_all_test(void)
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
	tmp = load_inodes_all(&fs1);

	for (i = 0; i < 3; ++i)
		if (tmp[i].id != inodes[i].id)
			return false;

	free(tmp);
	return true;
}

bool fs_close_test(void)
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
		if (tmp->size != inodes[i].size)
			return false;
		if (tmp->id != inodes[i].id)
			return false;
		if (tmp->check_size != inodes[i].check_size)
			return false;
		if (tmp->creation_date != inodes[i].creation_date)
			return false;
		if (tmp->last_modified != inodes[i].last_modified)
			return false;
		if (tmp->location != inodes[i].location)
			return false;
		if (tmp->inode_offset != inodes[i].inode_offset)
			return false;
		if (tmp->custody != inodes[i].custody)
			return false;
		if (tmp->time_to_live != inodes[i].time_to_live)
			return false;
		buffer += fs1.sector_size;
	}

	free(tmp);
	buffer -= fs1.sector_size * 3;
	free(buffer);
	return true;
}

bool fs_open_test(void)
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

		if (tmp->size != inodes[i].size)
			return false;
		if (tmp->id != inodes[i].id)
			return false;
		if (tmp->check_size != inodes[i].check_size)
			return false;
		if (tmp->creation_date != inodes[i].creation_date)
			return false;
		if (tmp->last_modified != inodes[i].last_modified)
			return false;
		if (tmp->location != inodes[i].location)
			return false;
		if (tmp->inode_offset != inodes[i].inode_offset)
			return false;
		if (tmp->custody != inodes[i].custody)
			return false;
		if (tmp->time_to_live != inodes[i].time_to_live)
			return false;
	}

	free(tmp);
	free(buffer);
	return true;
}

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

bool fs_mkfs_test(void)
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
	disk_shutdown(disk1);
	disk_shutdown(disk2);
	disk_shutdown(disk3);
	disk_shutdown(disk4);
}

void run_fs_tests(void)
{
	uint i, length;

	bool (*tests[14]) (void);
	tests[0] = fs_mkfs_test;
	tests[1] = fs_mount_test;
	tests[2] = free_disk_space_test;
	tests[3] = isNotValid_test;
	tests[4] = load_inodes_all_test;
	tests[5] = delete_invalid_inodes_test;
	tests[6] = find_first_IN_length_test;
	tests[7] = fs_getfree_test;
	tests[8] = fs_write_test;
	tests[9] = fs_read_test;
	tests[10] = fs_delete_test;
	tests[11] = fs_close_test;
	tests[12] = fs_open_test;
	tests[13] = fs_create_test;

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
