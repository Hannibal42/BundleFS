/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the functions that manipulate/search the
 * allocation tables.
 */

#ifndef BIT_FUNCTIONS_H
#define BIT_FUNCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "file_system_structs.h"
#include "disk.h"
#include "disk_interface.h"
#include "hardware_specifics.h"
#include "window_buffer.h"

/**
 * Writes sets the specified bit to one or zero in the allocation table
 * @param[out] table The allocation table that is manipulated
 * @param[in] index The index of the bit in bits
 * @param[in] value The value the bit should have
 */
void write_bit(uint8_t *table, uint index, bool value);
/**
 * Searches the table from left to right for the first bit that is zero
 * @param[in] table The allocation table that is searched
 * @param[in] table_size The size of the table in byte
 * @return The index of the first free bit in bits, -1 if no bit was found,
 * -2 if an error occurred
 */
int find_bit(const uint8_t *table, uint table_size);
/**
 *  Searches the table from left to right for the first sequence of zero bit
 *  that has the given length.
 *  @param[in] table The allocation table that is searched
 *  @param[in] table_size The size of the table in byte
 *  @param[in] length The length of the sequence in bit
 *  @return The start index of the sequence in bit, or -1 if no sequence was found.
 */
//int find_seq(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 'length' bits starting from 'index' */
//void write_seq(uint8_t *table, uint index, uint length);
/* Deletes a sequence of 'length' bits starting from 'index' */
//void delete_seq(uint8_t *table, uint index, uint length);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
uint check_size(void);
int find_seq_small(const uint8_t *table, uint table_size, uint length);
unsigned long div_up(unsigned long dividend, unsigned long divisor);

void con32to8(uint8_t *arr, uint32_t value);
uint32_t con8to32(uint8_t *arr);


/**
 * Deletes a sequence in the table.
 * @param[in] win The window struct holding the buffer.
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the delete was successful, false if the index is out of range, or the buffer could not be saved.
 */
bool delete_seq_global(struct AT_WINDOW *win, uint index, uint length);
/**
 * Writes a sequence of 1 into the table.
 * @param[in] win The window  struct holding the buffer
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the write was successful, false if the index is out of range, or the buffer could not be saved.
 */
bool write_seq_global(struct AT_WINDOW *win, uint index, uint length);
/**
 * Checks if a sequence of bits is 0.
 * @param[in] win The window  struct holding the buffer
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the sequence is zero, false if the sequence is not zero, or some error occurred.
 */
bool check_seq_global(struct AT_WINDOW *win, uint index, uint length);
/**
 * Searches the table from left to right for the first sequence of 0s of the specified length.
 * @param[in] win The window struct holding the buffer
 * @param[in] length The length of the sequence in bit.
 * @param[out] index The start index of the sequence in bit, -1 if no sequence was found.
 * @return True if a sequence was found, false if no sequence was found, or if an error occurred.
 */
bool find_seq_global(struct AT_WINDOW *win, uint length, uint *index);
/**
 * Finds the largest sequence of 0s in the table.
 * @param[in] win The window strut holding the buffer.
 * @param[out] length The lenght of the seqence found.
 * @param[out] index The start bit of the sequence found.
 * @return True if a sequence was found, false if an error occurred.
 */
bool find_largest_seq_global(struct AT_WINDOW *win, uint *length, uint *index);

#endif /* BIT_FUNCTIONS_H */
