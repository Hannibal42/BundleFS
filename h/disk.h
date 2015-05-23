#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>

enum DISK_STATUS {
	STA_NOINIT,/*Drive is not initialized*/
	STA_NODISK,/*No Medium*/
	STA_PROTECT,/*Medium is write protected*/
	STA_READY,
	STA_ERROR_NO_FILE,
};

struct disk {
	char number;
	unsigned long size;/*size of the device in byte*/
	char *file_name;
	FILE *file;
	uint sector_size;
	uint sector_count;
	enum DISK_STATUS status;
};

/*Fills the struct with the right parameters*/
struct disk *disk_fill(char *filename, uint size, uint sector_size);
/*Creates the file for the disk*/
void disk_create(struct disk *disk, uint size);

#endif /* DISK_H */
