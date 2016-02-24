#ifndef INCLUDE_INODE_FUNCTIONS_H_
#define INCLUDE_INODE_FUNCTIONS_H_

#include <stdbool.h>
#include <string.h>

#include "bit_functions.h"
#include "file_system_structs.h"

void quicksort_inodes(struct INODE *inodes, int nitems);
/* checks if the sequence of a given length starting at index is all 0 */
bool check_seq(uint8_t *table, uint index, uint length);
/* Finds the first inode with a minimum lenght that can be deleted */
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size);
/* Returns true if the inode TTL is expired */
bool isNotValid(struct INODE *inode);
/* Returns how many inodes are in use right now */
uint inodes_used(struct FILE_SYSTEM *fs);
/* Writes the given inode to disk */
void write_inode(struct FILE_SYSTEM *fs, struct INODE *file);

void load_inodes_block(struct FILE_SYSTEM *fs, struct INODE *buffer);

void load_inode_block(struct FILE_SYSTEM *fs, struct INODE *buffer,
	uint *pos, uint ino_cnt, uint sec_num);
void get_ino_pos(struct FILE_SYSTEM *fs, uint8_t *in_tab,
	uint offset, uint *pos, uint *ino_cnt);

#endif /* INCLUDE_INODE_FUNCTIONS_H_ */
