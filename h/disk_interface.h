#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <stdio.h>
#include <time.h>
#include "disk.h"
/* Command code for disk_ioctrl function */

/* Generic command (used by FatFs) */
/* Flush disk cache (for write functions) */
#define CTRL_SYNC			0
/* Get media size (for only f_mkfs()) */
#define GET_SECTOR_COUNT	1
/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_SECTOR_SIZE		2
/* Get erase block size (for only f_mkfs()) */
#define GET_BLOCK_SIZE		3
/* Force erased a block of sectors (for only _USE_ERASE) */
#define CTRL_ERASE_SECTOR	4

enum DRESULT {
	RES_OK,
	RES_ERROR,
	RES_PARERR,/*Invalid Parameter*/
	RES_NOTRDY/*Not ready*/
};

enum DRESULT disk_create(struct disk *disk,
uint size);/*TODO: shouldnt be here...*/
enum DISK_STATUS disk_status(const struct disk *disk);
enum DRESULT disk_initialize(struct disk *disk);
enum DRESULT disk_shutdown(struct disk *disk);
enum DRESULT disk_read(struct disk *disk, char *buff, unsigned long sector,
	unsigned long number_of_sectors);
enum DRESULT disk_write(struct disk *disk, char *buff, unsigned long sector,
	unsigned long number_of_sectors);
enum DRESULT disk_ioctl(struct disk *disk, char cmd, unsigned long *buff);
time_t system_get_time(void);

#endif /* DISK_INTERFACE_H */
