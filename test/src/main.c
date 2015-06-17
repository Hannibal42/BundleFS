#include "main.h"
#include "metadata.h"

int main(void)
{
	printf("%lu\n", sizeof(struct INODE));
	printf("\n-------------------utility_tests------------------\n");
	RUN_TEST_GROUP(utility_tests);
	printf("\n---------------------fs_tests---------------------\n");
	RUN_TEST_GROUP(fs_tests);
	printf("\n---------------------tasks_tests---------------------\n");
	RUN_TEST_GROUP(tasks_tests);
	printf("\n--------------------system_tests-------------------\n");
	RUN_TEST_GROUP(system_tests);
	printf("\n");
	return 0;
}

/*
int main(void)
{
	int i;
	char *buffer;
	struct disk *disk, *disk2;
	struct INODE *inode1, *inode2, *inode3, *inode4;
	struct FILE_SYSTEM *fs;

	disk = disk_fill("test.disk", 1024, 64);
	disk_create(disk, 1024);
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
	inode2 = (struct INODE *) malloc(sizeof(struct INODE));
	fs_open(fs, 1, inode2);
	print_inode(inode2);
	fs_delete(fs, inode1);

	free(inode1);
	free(inode2);
	free(inode3);
	free(inode4);

	print_fs(fs);
	printf("Free: %lu\n", fs_getfree(disk2, fs));

	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0;
}*/
