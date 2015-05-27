#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "disk_interface.h"


/* Standart start point for the fs */

#define FS_START 0 /* TODO: Remove? */

/* File system information */
struct FILE_SYSTEM {
	ushort id;
	uint sector_size;
	uint sector_count;
	/* Pointer to the disk the fs is belonging to */
	struct disk *disk;
	/* Start of the allocation table */
	uint alloc_table;
	uint alloc_table_size;
	/* Start of the inode allocation table */
	uint inode_alloc_table;
	uint inode_alloc_table_size;
	/* Start of the inode block */
	uint inode_block;
	uint inode_block_size;
};

/* File information */
struct INODE {
	/* unique id in the file system */
	uint id;
	uint size;
	uint check_size;
	uint creation_date;
	uint last_modified;
	/* Offset into the file beginning from the start */
	uint offset; /* TODO: Maybe remove */
	/* Pointer to the start sector of the file */
	uint location;
	uint inode_offset;
	/* The custody of the bundle */
	bool custody;
	/* The TTL of the bundle */
	uint time_to_live;
};

/* Modes for fs_seek */
enum SEEK_MODE {
	CUR,
	SET,
	END
};

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
/* Reads the fs from  */
enum FSRESULT fs_mount(struct disk *disk, struct FILE_SYSTEM *fs);
/* Returns how many sectors are still free */
unsigned long fs_getfree(struct disk *disk, struct FILE_SYSTEM *fs);

/*enum FSRESULT fs_flush(); //Writes all cached data to the disk*/
/*unsigned long fs_tell(struct INODE* file);*/
/*enum FSRESULT fs_seek(struct INODE* file,int offset, enum SEEK_MODE mod);*/

#endif /* FILE_SYSTEM_H */
