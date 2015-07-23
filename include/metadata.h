#ifndef METADATA_H
#define METADATA_H

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

/* File system information */
struct FILE_SYSTEM {
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
	/* Maximal number of inodes */
	uint inode_max;
	/* Number of inodes per sector */
	uint inode_sec;
};

/* File information */
struct INODE {
	/* The custody of the bundle */
	bool custody;
	/* The size of the file in byte */
	uint size;
	/* Pointer to the start sector of the file */
	uint location;
	/* Offset to the inode location */
	uint16_t inode_offset;
	/* The TTL of the bundle */
	uint time_to_live;
};

#endif /* METADATA_H */
