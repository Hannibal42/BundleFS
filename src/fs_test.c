#include "../h/fs_test.h"


struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;




void setUp(void)
{
	/*Struct setup*/
	disk1 = make_disk("disk1.disk", 1024, 64);
	disk2 = make_disk("disk2.disk", 2048, 128);
	disk3 = make_disk("disk3.disk", 4096, 256);
	disk4 = make_disk("disk4.disk", 65536, 512);

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


int main(void)
{
	setUp();
	if (mkfs_test() == 0)
		printf("Fehler!");
	tearDown();
}
