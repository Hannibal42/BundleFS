#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <stdio.h>
#include <time.h>
#include "disk.h"

enum DRESULT{
	RES_OK,
	RES_ERROR,
	RES_PARERR, //Invalid Parameter
	RES_NOTRDY //Not ready
};

enum DISK_STATUS disk_status(const struct disk* disk);
enum DRESULT disk_initialize(struct disk* disk);
enum DRESULT disk_shutdown(struct disk* disk);
enum DRESULT disk_read(struct disk* disk, char* buff, int block, int number_of_blocks);
enum DRESULT disk_write(struct disk* disk, char* buff, int block, int number_of_blocks);
//void disk_ioctl(char[3] command);
time_t system_get_time();

#endif /* DISK_INTERFACE_H */