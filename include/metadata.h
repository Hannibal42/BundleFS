#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

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
	/* Pointer to the start sector of the file */
	uint location;
	uint inode_offset;
	/* The custody of the bundle */
	bool custody;
	/* The TTL of the bundle */
	uint time_to_live;
};

#endif /* METADATA_H */
