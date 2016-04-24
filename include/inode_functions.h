/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the function signatures for the functions that are used
 * to write, load, find and sort inodes in the file system.
 */

#ifndef INCLUDE_INODE_FUNCTIONS_H_
#define INCLUDE_INODE_FUNCTIONS_H_

#include <stdbool.h>
#include <string.h>

#include "bit_functions.h"
#include "file_system_structs.h"
#include "window_buffer.h"

/**
 * Sorts the inodes by its location ascending with quicksort
 * @param[in] inodes The inodes to be sorted
 * @param[in] nitems The number of inodes to be sorted
 */
void quicksort_inodes(struct INODE *inodes, int nitems);

/*
 *  Finds the first inode with a minimum lenght that can be deleted
 * 	@param[in] fs The struct holding the file system metadata
 * 	@param[out] file The inode that can be deleted
 * 	@param[in] size The size in byte that the inode needs to have
 * */
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size);

/**
 *  Returns true if the inode TTL is expired
 *  @param[in] inode The inode to be checked
 */
bool isNotValid(struct INODE *inode);

/**
 * Returns how many inodes are in use right now
 * @param[in] fs The struct holding the file system metadata
 **/
uint inodes_used(struct FILE_SYSTEM *fs);

/**
 * Writes the given inode to disk.
 * @param[in] fs The struct holding the file system metadata
 * @param[in] file The inode that should be written to disk
 **/
void write_inode(struct FILE_SYSTEM *fs, struct INODE *file);

/**
 *  Loads all inodes into the buffer, the inodes are loaded sector by sector
 *	@param[in] fs The struct holding the file system metadata
 *	@param[out] buffer The buffer with all inodes.
 **/
void load_all_inodes(struct FILE_SYSTEM *fs, struct INODE *buffer);

/**
 *  Loads specified inodes in a sector, the functions needs the position
 *  of all valid inodes and in ino_cnt the number of valid inodes
 *  @param[in] fs The struct holding the file system metadata
 *  @param[out] buffer The buffer with the loaded inodes
 *  @param[in] pos The array that contains the positions of the inodes to be
 *   loaded
 *  @param[in] ino_cnt The total number of inodes to be loaded
 *  @param[in] sec_num The number of the sector where the inodes are
 */
void load_inode_block(struct FILE_SYSTEM *fs, struct INODE *buffer,
	uint *pos, uint ino_cnt, uint sec_num);

/**
 * Returns the position of all valid inodes in a sector
 * @param[in] fs The struct holding the file system metadata
 * @param[in] offset The offset into the inode allocation table
 * @param[out] pos The array containing the positions of the inodes
 * @param[out] ino_cnt The number of inodes found
 */
void get_ino_pos_new(struct FILE_SYSTEM *fs,
	uint offset, uint *pos, uint *ino_cnt);

#endif /* INCLUDE_INODE_FUNCTIONS_H_ */
