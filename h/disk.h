#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>

enum DISK_STATUS{
	STA_NOINIT, //Drive is not initialized
	STA_NODISK, //No Medium 
	STA_PROTECT, //Medium is write protected
	STA_READY
};

struct disk {
	char number;
	int size; //size of the device in byte
	char* file_name;
	FILE* file;
	int block_size;
	enum DISK_STATUS status;
};

struct disk* make_disk(char* filename);

#endif /* DISK_H */