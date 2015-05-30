#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "disk.h"
#include "metadata.h"

/*Writes the given bit into the allocation table*/
void write_bit(uint8_t *table, uint index, bool value);
/* Finds the first free bit */
int find_bit(const uint8_t *table, uint length);
/* Finds the first free sequence of the given length */
int find_sequence(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
void write_seq(uint8_t *table, uint index, uint length);
/* Lets the bits toggle */
void delete_seq(uint8_t *table, uint index, uint length);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
void checksum(const uint8_t *buffer, uint lenght, uint8_t *result, uint size);
bool checksum_check(const uint8_t *buffer, const struct INODE *file,
uint sector_size);
int find_sequence_small(const uint8_t *table, uint table_size, uint length);
inline unsigned long div_up(unsigned long dividend,
	unsigned long divisor);

#endif /* UTILITY_H */