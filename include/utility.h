#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

#include <string.h>
#include <stdbool.h>
#include "disk.h"
#include "metadata.h"
#include "disk_interface.h"
#include "board_specifics.h"

/*Writes the given bit into the allocation table*/
void write_bit(uint8_t *table, uint index, bool value);
/* Finds the first free bit */
int find_bit(const uint8_t *table, uint length);
/* Finds the first free sequence of the given length */
int find_seq(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
void write_seq(uint8_t *table, uint index, uint length);
/* TODO Lets the bits toggle */
void delete_seq(uint8_t *table, uint index, uint length);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
uint check_size(void);
int find_seq_small(const uint8_t *table, uint table_size, uint length);
unsigned long div_up(unsigned long dividend,
	unsigned long divisor);
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
void con32to8(uint8_t *arr, uint32_t value);
uint32_t con8to32(uint8_t *arr);


void delete_seq_global(struct FILE_SYSTEM *fs, uint index, uint length);

#endif /* UTILITY_H */
