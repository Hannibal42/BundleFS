/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the struct that represents the physical disk.
 */

#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>

/**
 * The different states the physical Disk can have
 */
enum DISK_STATUS {
	/** The Medium is not initialized */
	STA_NOINIT,
	/** No Medium */
	STA_NODISK,
	/** The Medium is write protected */
	STA_PROTECT,
	/** The Medium is ready */
	STA_READY,
	STA_ERROR_NO_FILE,
};

/**
 * The struct representing the physical disk.
 */
struct DISK {
	/**
	 * Number of the drive
	 */
	char number;
	/**
	 * The size of the disk in byte
	 */
	unsigned long size;
	/**
	 * The name of the file that
	 * is used to simulate the disk.
	 */
	char *file_name;
	/**
	 * The file handle
	 */
	FILE *file;
	/**
	 * The size of one physical block
	 * on the disk in byte
	 */
	uint block_size;
	/**
	 * The number of blocks that the disk has.
	 */
	uint block_count;
	/**
	 * The number of blocks that makes up
	 * one logical sector of the file system
	 */
	uint sector_block_mapping;
	/**
	 * The status of the disk
	 */
	enum DISK_STATUS status;
};

/**
 * Fills the struct with the right parameters
 * @param[out] disk The Pointer to the disk struct that should be filled
 * @param[in] filename Name of the file that is used to simulate the disk
 * @param[in] size The size of the disk in byte
 * @param[in] block_size The size of one block on the disk in byte
 */
void disk_fill(struct DISK *disk, char *filename, uint size, uint block_size);
/**
 * Creates a new file for a given disk
 * @param[in] disk The disk that should be created
 * @param[in] size The size of the file in byte
 */
void disk_create(struct DISK *disk, uint size);

#endif /* DISK_H */
