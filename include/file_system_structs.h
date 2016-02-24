#ifndef FILE_SYSTEM_STRUCTS_H
#define FILE_SYSTEM_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

/* File system information */
struct FILE_SYSTEM {
	uint32_t sector_size;
	uint32_t sector_count;
	/* Pointer to the disk the fs is belonging to */
	struct DISK *disk;
	/* Start of the allocation table */
	uint32_t alloc_table;
	uint32_t alloc_table_size;
	/* The size of the allocation table buffer */
	uint32_t alloc_table_buffer_size;
	/* The buffer window for the allocation table */
	struct AT_WINDOW *at_win;
	/* Start of the inode allocation table */
	uint32_t inode_alloc_table;
	uint32_t inode_alloc_table_size;
	/* Start of the inode block */
	uint32_t inode_block;
	uint32_t inode_block_size;
	/* Maximal number of inodes */
	uint32_t inode_max;
	/* Number of inodes per sector */
	uint32_t inode_sec;
};

/* File information */
struct INODE {
	/* The custody of the bundle */
	bool custody;
	/* The size of the file in byte */
	uint32_t size;
	/* Pointer to the start sector of the file */
	uint32_t location;
	/* Offset to the inode location */
	uint16_t inode_offset;
	/* The TTL of the bundle */
	uint32_t time_to_live;
};

#endif /* FILE_SYSTEM_STRUCTS_H */
