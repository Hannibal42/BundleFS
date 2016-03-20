/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the interface that is used to access the block device.
 * Allows to read/write the disk
 */

#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "disk.h"

/** Flush disk cache (for write functions) */
#define CTRL_SYNC			0
/** Get media size (for only f_mkfs()) */
#define GET_SECTOR_COUNT	1
/** Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_SECTOR_SIZE		2
/** Get erase block size (not working right now) */
#define GET_BLOCK_SIZE		3
/* Force erased a block of sectors (for only _USE_ERASE) */
#define CTRL_ERASE_SECTOR	4

/**
 * The results of the disk_interface actions.
 */
enum DRESULT {
	RES_OK,
	RES_ERROR,
	/**
	 * Invalid Parameter
	 */
	RES_PARERR,
	/**
	 * Not ready
	 */
	RES_NOTRDY
};


/**
 * Returns the status of the disk
 * @param[in] disk The struct representing the disk
 * @return The Status of the disk
 */
enum DISK_STATUS disk_status(const struct DISK *disk);
/**
 * Initializes the disk, must be done before the first call
 * to the other disk_interface functions.
 * @param[in] disk The struct representing the disk
 * @return The result of the initialization
 */
enum DRESULT disk_initialize(struct DISK *disk);
/**
 * Shutsdown the disk, should be called when the disk is
 * no longer needed.
 * @param[in] disk The struct representing the disk
 */
enum DRESULT disk_shutdown(struct DISK *disk);
/**
 * Reads the specified number of sectors from the disk and
 * stores them in the buffer.
 * @param[in] disk The struct representing the disk
 * @param[out] buff The buffer that is used to store the content of the sectors
 * @param[in] sector The first sector of the sequence that should be read
 * @param[in] number_of_sectors The number of sectors that should be read
 */
enum DRESULT disk_read(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors);
/**
 * Writes the buffer to the specified sequence of sectors
 * @param[in] disk The struct representing the disk
 * @param[out] buff The buffer that is used to store the content
 * @param[in] sector The first sector of the sequence that should be written
 * @param[in] number_of_sectors The number of sectors that should be written
 */
enum DRESULT disk_write(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors);
/**
 * Returns information about the disk, see the defines for cmd
 * @param[in] disk The struct representing the disk
 * @param[in] cmd The command
 */
enum DRESULT disk_ioctl(struct DISK *disk, char cmd, unsigned long *buff);

/**
 * Returns the current system time.
 */
time_t system_get_time(void);

#endif /* DISK_INTERFACE_H */
