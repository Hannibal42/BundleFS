#ifndef INCLUDE_INODE_FUNCTIONS_H_
#define INCLUDE_INODE_FUNCTIONS_H_

#include <stdbool.h>
#include <string.h>

#include "bit_functions.h"
#include "file_system_structs.h"
#include "window_buffer.h"

/**
 * Sorts the inodes by its location ascending with quicksort
 * [in] inodes The inodes to be sorted
 * [in] nitems The number of inodes to be sorted
 */
void quicksort_inodes(struct INODE *inodes, int nitems);

/*
 *  Finds the first inode with a minimum lenght that can be deleted
 * 	[in] fs The struct holding the file system metadata
 * 	[out] file The inode that can be deleted
 * 	[in] size The size in byte that the inode needs to have
 * */
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size);
/**
 *  Returns true if the inode TTL is expired
 *  [in] inode The inode to be checked
 */
bool isNotValid(struct INODE *inode);
/**
 * Returns how many inodes are in use right now
 * [in] fs The struct holding the file system metadata
 **/
uint inodes_used(struct FILE_SYSTEM *fs);
/**
 * Writes the given inode to disk
 * [in] fs The struct holding the file system metadata
 **/
void write_inode(struct FILE_SYSTEM *fs, struct INODE *file);

/**
 * Loads the inodes from all blocks
 */
void load_inodes_block(struct FILE_SYSTEM *fs, struct INODE *buffer);

void load_inode_block(struct FILE_SYSTEM *fs, struct INODE *buffer,
	uint *pos, uint ino_cnt, uint sec_num);
void get_ino_pos(struct FILE_SYSTEM *fs, uint8_t *in_tab,
	uint offset, uint *pos, uint *ino_cnt);

#endif /* INCLUDE_INODE_FUNCTIONS_H_ */
