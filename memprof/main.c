#include "main.h"

int main(void)
{
	int i;
	struct FILE_SYSTEM fs;
	struct disk disk;
	struct INODE fil;

	disk_fill(&disk, "disks/disk4.disk", 104857600, 4096);

	disk_create(&disk,  104857600);
	disk_initialize(&disk);
	fs_mkfs(&disk);

	fs_mount(&disk, &fs);

	uint8_t *buf;

	buf = malloc(fs.sector_size);

	for (i = 0; i < fs.sector_size; ++i) {
		buf[i] = 0xFF;
	}

	fs_create(&fs, &fil, fs.sector_size, 1, true);
	fs_write(&fs, &fil, (char *) buf );

	free(buf);

	buf = malloc(fs.sector_size);

	fs_read(&fs, &fil, (char *) buf, fs.sector_size);

	fs_delete(&fs, &fil);

	for (i = 0; i < 100; ++i) {
		fil.id = i;
		fs_create(&fs, &fil, fs.sector_size, 1, true);
		fs_write(&fs, &fil, (char *) buf);
		fs_read(&fs, &fil, (char *) buf, fs.sector_size);
	}

	for (i = 0; i < 100; ++i)
		if (fs_open(&fs, i * 2, &fil) == FS_OK)
			fs_delete(&fs, &fil);

	printf("Defragment \n");
	defragment(&fs);
	printf("Delete Invalid Inodes \n");
	delete_invalid_inodes(&fs);

	/*
	for (i = 0; i < 100; ++i) {
		fil.id = i;
		fs_create(&fs, &fil,2 * fs.sector_size, 1, true);
		fs_write(&fs, &fil, (char *) buf);
		fs_read(&fs, &fil, (char *) buf,2 * fs.sector_size);
	}*/

	return 0;
}