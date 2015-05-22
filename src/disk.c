#include "../h/disk.h"

struct disk *make_disk(char *filename, uint size, uint sector_size)
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
