#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "disk_interface.h"
#include "metadata.h"
#include "utility.h"

/* Results of the file functions */
enum FSRESULT {
	FS_OK,
	FS_ERROR,
	FS_PARAM_ERROR,
	FS_FULL,
	FS_CHECK_ERROR
};

/* Reads the inode from the disk */
enum FSRESULT fs_open(struct FILE_SYSTEM *fs, uint number, struct INODE *file);
/* Creates a new inode and allocates the space for the data */
enum FSRESULT fs_create(struct FILE_SYSTEM *fs, struct INODE *file,
unsigned long size, uint time_to_live, bool custody);
/* Closes the file and writes all changes to the disk */
enum FSRESULT fs_close(struct FILE_SYSTEM *fs, struct INODE *file);
/* Deletes the file */
enum FSRESULT fs_delete(struct FILE_SYSTEM *fs, struct INODE *file);
/* Reads the file from disk */
enum FSRESULT fs_read(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer, uint length);
/* Writes the file to the disk */
enum FSRESULT fs_write(struct FILE_SYSTEM *fs, struct INODE *file,
	char *buffer);
/* Creates the ondisk structures for the file system */
enum FSRESULT fs_mkfs(struct disk *disk);
/* Reads the fs from the given disk*/
enum FSRESULT fs_mount(struct disk *disk, struct FILE_SYSTEM *fs);
/* Returns how many sectors are still free */
unsigned long fs_getfree(struct FILE_SYSTEM *fs);

#endif /* FILE_SYSTEM_H */
