#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <stdio.h>
#include <stdint.h>
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


/*Returns the status of the disk*/
enum DISK_STATUS disk_status(const struct DISK *disk);
/*Opens the filehandler for the disk*/
enum DRESULT disk_initialize(struct DISK *disk);
/*Closes the filehandler*/
enum DRESULT disk_shutdown(struct DISK *disk);
/*Reads from sector and stores the result in buff*/
enum DRESULT disk_read(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors);
/*Writes buff starting from sector*/
enum DRESULT disk_write(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors);
/*Returns information about the disk, see the defines for cmd*/
enum DRESULT disk_ioctl(struct DISK *disk, char cmd, unsigned long *buff);
/*Returns the system time*/
time_t system_get_time(void);

#endif /* DISK_INTERFACE_H */
