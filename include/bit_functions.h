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

/*Writes the given bit into the allocation table*/
void write_bit(uint8_t *table, uint index, bool value);
/* Finds the first free bit */
int find_bit(const uint8_t *table, uint length);
/* Finds the first free sequence of the given length */
int find_seq(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 'length' bits starting from 'index' */
void write_seq(uint8_t *table, uint index, uint length);
/* Deletes a sequence of 'length' bits starting from 'index' */
void delete_seq(uint8_t *table, uint index, uint length);
int first_free_bits(uint8_t byte);
int last_free_bits(uint8_t byte);
int get_free_bit(uint8_t index, uint8_t byte);
int popcount(uint8_t byte);
uint check_size(void);
int find_seq_small(const uint8_t *table, uint table_size, uint length);
unsigned long div_up(unsigned long dividend, unsigned long divisor);

void con32to8(uint8_t *arr, uint32_t value);
uint32_t con8to32(uint8_t *arr);


bool delete_seq_global(struct AT_WINDOW *win, uint index, uint length);
bool write_seq_global(struct AT_WINDOW *win, uint index, uint length);
bool find_seq_global(struct AT_WINDOW *win, uint table_size, uint length, uint *index);
bool check_seq_global(struct AT_WINDOW *win, uint index, uint length);

#endif /* BIT_FUNCTIONS_H */
