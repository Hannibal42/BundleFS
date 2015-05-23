#include "../h/fs_test.h"


struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;

extern void write_alloc_table(uint index, uint8_t *table, bool value);


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

	fs_mount(disk1, &fs1);
	struct INODE file;
	fs_create(&fs1, &file, 80, 100,true);
	print_fs(&fs1);
	print_inode(&file);
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


int write_alloc_table_test(void)
{
	uint8_t *test_table;

	test_table = malloc(sizeof(uint8_t) * 5);
	test_table[0] = 0xFF;
	test_table[1] = 0XF8;
	test_table[2] = 0x00;
	test_table[3] = 0xFF;
	test_table[4] = 0xFF;

	write_alloc_table(0, test_table, 0);
	if (test_table[0] != 0x7F)
		return false;

	write_alloc_table(0, test_table, 1);
	if (test_table[0] != 0xFF)
		return false;

	write_alloc_table(8, test_table, 1);
	if (test_table[1] != 0xF8)
		return false;

	write_alloc_table(8, test_table, 0);
	if (test_table[1] != 0x78)
		return false;

	write_alloc_table(18, test_table, 1);
	if (test_table[2] != 0x20)
		return false;

	write_alloc_table(39, test_table, 0);
	if (test_table[4] != 0xFE)
		return false;

	return true;
}

/*
int main(void)
{
	setUp();
	if (!mkfs_test())
		printf("Fehler in mkfs!\n");
	tearDown();
	if (!write_alloc_table_test())
		printf("Fehler in write_inode_alloc_table\n");
	return 0;
} */


int main(void)
{
	int i;
	char *buffer;
	struct disk *disk;
	struct disk *disk2;
	struct INODE *inode1;
	struct INODE *inode2;
	struct INODE *inode3;
	struct INODE *inode4;
	struct FILE_SYSTEM *fs;

	disk = disk_fill("test.disk", 1024, 64);
	disk_create(disk, 1024); //creates the file for the disk
	disk_initialize(disk);
	print_disk(disk);

	fs_mkfs(disk);
	disk_shutdown(disk);
	disk2 = disk_fill("test.disk", 1024, 64);
	disk_initialize(disk2);

	fs = malloc(sizeof(struct FILE_SYSTEM));
	fs->disk = disk;
	fs_mount(disk2, fs);
	fs->disk = disk2;


	inode1 = (struct INODE *) malloc(sizeof(struct INODE));
	inode2 = (struct INODE *) malloc(sizeof(struct INODE));
	inode3 = (struct INODE *) malloc(sizeof(struct INODE));
	inode4 = (struct INODE *) malloc(sizeof(struct INODE));
	inode1->id = 0;
	inode2->id = 1;
	inode3->id = 2;
	inode4->id = 3;

	fs_create(fs, inode1, 30, 1000, 1);
	fs_create(fs, inode2, 30, 1000, 1);
	fs_create(fs, inode3, 80, 1000, 1);
	fs_create(fs, inode4, 30, 1000, 1);

	free(inode1);

	inode1 = NULL;

	inode1 = (struct INODE *) malloc(sizeof(struct INODE));
	fs_open(fs, 0, inode1);

	buffer = malloc(fs->sector_size * 2);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x0F;
	fs_write(fs, inode1, buffer);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x11;
	fs_write(fs, inode2, buffer);

	for (i = 0; i < (fs->sector_size * 2); ++i)
		buffer[i] = 0xFF;
	fs_write(fs, inode3, buffer);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x01;

	fs_write(fs, inode4, buffer);

	print_inode(inode1);
	print_inode(inode2);
	fs_delete(fs, inode1);

	free(inode1);
	free(inode2);
	free(inode3);
	free(inode4);
/*
	printf("Number: %d\n", get_first_free_bit(0xFF));
	printf("Number: %d\n", get_first_free_bit(0x7F));
	printf("Number: %d\n", get_first_free_bit(0x3F));
	printf("Number: %d\n", get_first_free_bit(0x1F));
	printf("Number: %d\n", get_first_free_bit(0x0F));
	printf("Number: %d\n", get_first_free_bit(0x07));
	printf("Number: %d\n", get_first_free_bit(0x03));
	printf("Number: %d\n", get_first_free_bit(0x01));
	printf("Number: %d\n", get_first_free_bit(0x00));

	printf("Number: %d\n", get_last_free_bit(0xFF));
	printf("Number: %d\n", get_last_free_bit(0xFE));
	printf("Number: %d\n", get_last_free_bit(0xFC));
	printf("Number: %d\n", get_last_free_bit(0xF8));
	printf("Number: %d\n", get_last_free_bit(0xF0));
	printf("Number: %d\n", get_last_free_bit(0xE0));
	printf("Number: %d\n", get_last_free_bit(0xC0));
	printf("Number: %d\n", get_last_free_bit(0x80));
	printf("Number: %d\n", get_last_free_bit(0x00));*/

/*printf("Inode size:%d\n", sizeof(struct INODE));*/

	print_fs(fs);
	printf("Free: %lu\n", fs_getfree(disk2, fs));

	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0;
}

