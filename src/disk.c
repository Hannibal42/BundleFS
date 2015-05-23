#include "../h/disk.h"

struct disk *disk_fill(char *filename, uint size, uint sector_size)
{
	struct disk *disk = (struct disk *) malloc(sizeof(struct disk));

	disk->size = size;
	disk->sector_size = sector_size;
	disk->sector_count = disk->size / disk->sector_size;
	disk->file_name = filename;
	disk->file = NULL;
	disk->status = STA_NOINIT;
	return disk;
}

void disk_create(struct disk *disk, uint size)
{

	if (!disk)
		return;

	disk->file = fopen(disk->file_name, "w+b");
	fseek(disk->file, 0, SEEK_SET);
	fclose(disk->file);
}
