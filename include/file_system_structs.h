/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the structs that represent the
 * file system on disk and in the main memory.
 */

#ifndef FILE_SYSTEM_STRUCTS_H
#define FILE_SYSTEM_STRUCTS_H

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

/**
 *  The main file system struct that represents the file system.
 *  When the file system is mounted the struct is loaded from disk, it contains
 *  all necessary information to access the file system.
 */
struct FILE_SYSTEM {
	/**
	 * The size of one logical sector of the file system in byte.
	 * One logical sector is always a multiple of the physical disk block size.
	 */
	uint32_t sector_size;
	/**
	 * The number of sectors that the fs manages.
	 */
	uint32_t sector_count;
	/**
	 *  Pointer to the struct that represents the physical disk
	 */
	struct DISK *disk;
	/**
	 * The index of the start sector of the allocation table
	 */
	uint32_t alloc_table;
	/**
	 * The size of the allocation table in sectors
	 */
	uint32_t alloc_table_size;
	/**
	 *  The size of the allocation table buffer in byte
	 */
	uint32_t alloc_table_buffer_size;
	/**
	 * The struct which holds the buffer that is used to access
	 * the allocation table and inode table.
	 */
	struct AT_WINDOW *at_win;
	/**
	 *  The index of the start sector of the inode allocation table.
	 */
	uint32_t inode_alloc_table;
	/**
	 * The size of the inode allocation table in sectors.
	 */
	uint32_t inode_alloc_table_size;
	/**
	 * The index of the start sector of the inode block
	 */
	uint32_t inode_block;
	/**
	 * The size of the inode block in sectors.
	 */
	uint32_t inode_block_size;
	/**
	 * The number of inodes that the file system can have with its
	 * current inode block.
	 */
	uint32_t inode_max;
	/**
	 * The number of inodes that fit into one sector
	 */
	uint32_t inode_sec;
};

/**
 * The struct that represents one bundle in the file system,
 * to access a bundle this struct is loaded from disk.
 * It contains all the information needed to access the content
 * of the bundle.
 */
struct INODE {
	/**
	 * Indicates of the bundle manager currently holds the custody
	 * for this bundle.
	 */
	bool custody;
	/**
	 * The size of the bundle in byte.
	 */
	uint32_t size;
	/**
	 * The index of the start sector of the bundle
	 */
	uint32_t location;
	/**
	 * The offset of the inode into the inode block
	 * in sizeof(struct INODE).
	 */
	uint16_t inode_offset;
	/**
	 * The time to live of the bundle, is used to
	 * identify invalid bundles.
	 */
	uint32_t time_to_live;
};

#endif /* FILE_SYSTEM_STRUCTS_H */
