#include "disk.h"

void disk_fill(struct DISK *disk, char *filename, uint size, uint sector_size)
{
	disk->size = size;
	disk->block_size = sector_size;
	disk->block_count = disk->size / disk->block_size;
	disk->file_name = filename;
	disk->file = NULL;
	disk->status = STA_NOINIT;
}

void disk_create(struct DISK *disk, uint size)
{

	if (!disk)
		return;

	disk->file = fopen(disk->file_name, "w+b");
	fseek(disk->file, 0, SEEK_SET);
	fclose(disk->file);
}
