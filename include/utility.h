#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"
#include "metadata.h"
#include "disk_interface.h"

/*Writes the given bit into the allocation table*/
void write_bit(uint8_t *table, uint index, bool value);
/* Finds the first free bit */
int find_bit(const uint8_t *table, uint length);
/* Finds the first free sequence of the given length */
int find_seq(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
void write_seq(uint8_t *table, uint index, uint length);
/* Lets the bits toggle */
void delete_seq(uint8_t *table, uint index, uint length);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
void checksum(const uint8_t *buffer, uint lenght, uint8_t *result, uint size);
uint checksum_size(uint size);
bool checksum_check(const uint8_t *buffer, const uint8_t *check,
	const struct INODE *file, uint sector_size);
int find_seq_small(const uint8_t *table, uint table_size, uint length);
inline unsigned long div_up(unsigned long dividend,
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
/* Reads the inode at file->location */
void read_inode(struct FILE_SYSTEM *fs, struct INODE *file);
/* Loads all inodes from disk and stores them in buffer */
void load_inodes(struct FILE_SYSTEM *fs, struct INODE *buffer);

#endif /* UTILITY_H */
