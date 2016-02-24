#include "main.h"

#include "../include/window_buffer.h"

int main(void)
{
	int i;
	struct FILE_SYSTEM fs;
	struct DISK disk;
	struct INODE fil;

	disk_fill(&disk, "disks/disk4.disk", 104857600, 4096);

	disk_create(&disk,  104857600);
	disk_initialize(&disk);
	fs_mkfs(&disk);

	fs_mount(&disk, &fs);

	uint8_t *buf;

	buf = SEC_BUFFER;

	for (i = 0; i < fs.sector_size; ++i) {
		buf[i] = 0xFF;
	}

	fs_create(&fs, &fil, fs.sector_size, 1, true);
	fs_write(&fs, &fil, buf );

	for (i = 0; i < fs.sector_size; ++i)
		buf[i] = 0x00;

	fs_read(&fs, &fil, buf, fs.sector_size);

	fs_delete(&fs, &fil);

	for (i = 0; i < 1000; ++i) {
		fil.inode_offset = i;
		fs_create(&fs, &fil, fs.sector_size, 1, true);
		fs_write(&fs, &fil, buf);
		fs_read(&fs, &fil, buf, fs.sector_size);
	}

	for (i = 0; i < 1000; ++i)
		if (fs_open(&fs, i * 2, &fil) == FS_OK)
			fs_delete(&fs, &fil);

	printf("Defragment \n");
	defragment(&fs);
	printf("Delete Invalid Inodes \n");
	delete_invalid_inodes(&fs);

	return 0;
}
