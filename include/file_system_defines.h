/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the size defines for the file system
 * like its logical sector size and the initial maximum number
 * of inodes.
 */

#ifndef FS_DEFINES_H
#define FS_DEFINES_H

/**
 * The size of the allocation table in byte
 */
#define AT_SIZE 16000
/**
 * The size of one logical sector in byte.
 */
#define SECTOR_SIZE 4098
/**
 *  The size of the allocation table buffer in byte
 *  (needs to be a multiple of SECTOR_SIZE)
 */
#define AT_BUFFER_SIZE 1 * SECTOR_SIZE
/*
 * The initial maximum number of inodes
 */
#define INODE_CNT 8

#endif /* FS_DEFINES_H */
