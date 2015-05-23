#include "../h/fs_test.h"


struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;

extern void write_inode_alloc_table(uint index, uint8_t *table, bool value);


void setUp(void)
{
	/*Struct setup*/
	disk1 = disk_fill("disk1.disk", 1024, 64);
	disk2 = disk_fill("disk2.disk", 2048, 128);
	disk3 = disk_fill("disk3.disk", 4096, 256);
	disk4 = disk_fill("disk4.disk", 65536, 512);

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
}



void tearDown(void)
{
	free(disk1);
	free(disk2);
	free(disk3);
	free(disk4);
}


int mkfs_test(void)
{

	return 1;
}


int write_inode_alloc_table_test(void)
{
	uint8_t *test_table;

	test_table = malloc(sizeof(uint8_t) * 5);
	test_table[0] = 0xFF;
	test_table[1] = 0XF8;
	test_table[2] = 0x00;
	test_table[3] = 0xFF;
	test_table[4] = 0xFF;

	write_inode_alloc_table(0, test_table, 0);
	if (test_table[0] != 0x7F)
		return false;

	write_inode_alloc_table(0, test_table, 1);
	if (test_table[0] != 0xFF)
		return false;

	write_inode_alloc_table(8, test_table, 1);
	if (test_table[1] != 0xF8)
		return false;

	write_inode_alloc_table(8, test_table, 0);
	if (test_table[1] != 0x78)
		return false;

	write_inode_alloc_table(18, test_table, 1);
	if (test_table[2] != 0x20)
		return false;

	write_inode_alloc_table(39, test_table, 0);
	if (test_table[4] != 0xFE)
		return false;

	return true;
}


int main(void)
{
	setUp();
	if (!mkfs_test())
		printf("Fehler in mkfs!\n");
	tearDown();
	if (!write_inode_alloc_table_test())
		printf("Fehler in write_inode_alloc_table\n");
	return 0;
}
