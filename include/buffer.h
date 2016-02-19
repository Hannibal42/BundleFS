#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "metadata.h"
#include "disk_interface.h"
#include "fs_defines.h"

/* Note: The unused warning is suppressed
for these buffers, works only for gcc */
static uint8_t __attribute__((__unused__)) AT_BUFFER[AT_BUFFER_SIZE];
static uint8_t __attribute__((__unused__)) IT_BUFFER[AT_SIZE];
static uint8_t __attribute__((__unused__)) SEC_BUFFER[SECTOR_SIZE];
static uint8_t __attribute__((__unused__)) INO_BUFFER[SECTOR_SIZE];

/* Initilizes the AT_WINDOW struct and loads the first window*/
bool init_window(struct AT_WINDOW *win, struct FILE_SYSTEM *fs, uint8_t *buffer);
/* Saves the content of the buffer and then moves the window and loads the new buffer */
bool move_window(struct AT_WINDOW *win, int index);
/* Saves the content of the buffer to disk */
bool save_window(struct AT_WINDOW *win);

#endif /* BUFFER_H */
