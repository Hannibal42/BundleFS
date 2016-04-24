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
 * Counts the population of a byte (set bits)
 * @param[in] byte The byte that should be counted
 * @returns The number of set bits in the byte
 */
int popcount(uint8_t byte);

/**
 * Calculates the div of the two numbers and rounds up
 * @param[in] dividend
 * @param[in] divisor
 */
unsigned long div_up(unsigned long dividend, unsigned long divisor);

/**
 * Splits a 32 bit number into 4 8 bit numbers
 * @param[out] arr The array with the resulting 4 bytes
 * @param[in] value The number that should be split
 */
void con32to8(uint8_t *arr, uint32_t value);

/**
 * Concatenates 4 Byte to a uint32_t
 * @param[in] arr The 4 bytes
 * @return The resulting uint32_t
 */
uint32_t con8to32(uint8_t *arr);


/**
 * Deletes a sequence in the table.
 * @param[in] win The window struct holding the buffer.
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the delete was successful, false if the index is out of
 * range, or the buffer could not be saved.
 */
bool delete_seq_global(struct AT_WINDOW *win, uint index, uint length);

/**
 * Writes a sequence of 1 into the table.
 * @param[in] win The window  struct holding the buffer
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the write was successful, false if the index is out of
 * range, or the buffer could not be saved.
 */
bool write_seq_global(struct AT_WINDOW *win, uint index, uint length);

/**
 * Checks if a sequence of bits is 0.
 * @param[in] win The window  struct holding the buffer
 * @param[in] index The index of the starting bit.
 * @param[in] length The length of the sequence in bit.
 * @return True if the sequence is zero, false if the sequence is not zero,
 * or some error occurred.
 */
bool check_seq_global(struct AT_WINDOW *win, uint index, uint length);

/**
 * Searches the table from left to right for the first sequence of 0s of the
 * specified length.
 * @param[in] win The window struct holding the buffer
 * @param[in] length The length of the sequence in bit.
 * @param[out] index The start index of the sequence in bit, -1 if no
 * sequence was found.
 * @return True if a sequence was found, false if no sequence was found,
 * or if an error occurred.
 */
bool find_seq_global(struct AT_WINDOW *win, uint length, uint *index);

/**
 * Finds the largest sequence of 0s in the table.
 * @param[in] win The window struct holding the buffer.
 * @param[out] start The start bit of the sequence found.
 * @param[out] length The lenght of the seqence found.
 * @return True if a sequence was found, false if an error occurred.
 */
bool find_max_sequence_global(struct AT_WINDOW *win, uint *start, uint *length);

#endif /* BIT_FUNCTIONS_H */
