/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the main interface for the file system.
 */


#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "bit_functions.h"
#include "file_system_structs.h"
#include "disk_interface.h"
#include "hardware_specifics.h"
#include "inode_functions.h"

/* Results of the file system functions */
enum FSRESULT {
	FS_OK,
	/* Generic error */
	FS_ERROR,
	/* The parameters of a function call are not valid  */
	FS_PARAM_ERROR,
	/** The file system is full */
	FS_FULL,
	/** The checksum for a sector does not match, the sector is corrupted */
	FS_CHECK_ERROR,
	/** Some error occurred reading/writing the disk */
	FS_DISK_ERROR
};

/**
 * Reads the specified inode from the disk
 * @param[in] fs The struct holding the file system metadata
 * @param[in] number The number of the inode requested
 * @param[out] file The loaded inode
 **/
enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number, struct INODE *file);
/**
 *  Creates a new file and allocates the space for the data
 *	@param[in] fs The struct holding the file system metadata
 *	@param[out] file The newly created inode
 *	@param[in] size The size of the new file in byte
 *	@param[in] time_to_live The time to live of the bundle
 *	@param[in] custody The bool that indicates if the bundle manager has the
 *	custody for the bundle.
 *
 **/
enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
	uint32_t size, uint64_t time_to_live, bool custody);
/**
 *  Closes the file and writes all changes to the disk
 *  @param[in] fs The struct holding the file system metadata
 *  @param[in] file The file that should be closed
 *
 **/
enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file);
/**
 *  Deletes the file
 * @param[in] file The file that should be deleted
 * @param[in] fs The struct holding the file system metadata
 **/
enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file);

/**
 * Reads the file from disk
 * @param[in] fs The struct holding the file system metadata
 * @param[in] file The file that should be read
 * @param[out] buffer The buffer that is used to read the bundle.
 * @param[in] length The number of bytes that shoud be read.
 *
 **/
enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	uint8_t *buffer, uint length);
/**
 *  Writes the buffer into a file on the disk
 *  @param[in] fs The struct holding the file system metadata
 *  @param[in] file The file that should be written to
 *  @param[in] buffer The buffer that should be written into the file
 **/
enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	uint8_t *buffer);

/**
 *  Creates the on-disk structures for the file system
 *  @param[in] disk The struct holding the metadata for the disk
 *  @param[in] sector_size The size of one sector in byte, must be a multiple
 *  of the disks block size.
 **/
enum FSRESULT fs_mkfs(struct DISK *disk, uint sector_size);

/**
 * Reads the file system from the given disk
 * @param[in] disk The struct holding the metadata for the disk
 * @param[out] fs The struct holding the file system metadata
 **/
enum FSRESULT fs_mount(struct DISK *disk, struct FILE_SYSTEM *fs);

/**
 *  Returns how many sectors are still free
 *  @param[in] fs The struct holding the file system metadata
 **/
unsigned long fs_getfree(struct FILE_SYSTEM *fs);

#endif /* FILE_SYSTEM_H */
