#include "../h/disk.h"

struct disk* make_disk(char* filename)
{
	struct disk* disk = (struct disk*) malloc(sizeof(struct disk));
	disk->size = 1024;
	disk->block_size = 64;
	disk->file_name = filename;
	disk->file = NULL;
	disk->status = STA_NOINIT;
	return disk;
}