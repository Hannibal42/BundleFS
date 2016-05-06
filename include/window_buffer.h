/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * Functions and definitions for the window buffer that is used by the file
 * system to access the allocation tables.
 */


#ifndef WINDOW_BUFFER_H
#define WINDOW_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "file_system_structs.h"
#include "disk_interface.h"
#include "file_system_defines.h"

/**
 * The struct that for the window buffer that is used to access the allocation
 * tables.
 */
struct AT_WINDOW {
	/**
	 * window is only valid if sectors are loaded
	 */
	bool isValid;
	/**
	 * The buffer of the window
	 */
	uint8_t *buffer;
	/**
	 * Index of the first sector that is currently in the buffer
	 */
	uint global_index;
	/**
	 * The fist sector of the allocation table that should be accessed
	 **/
	uint global_start;
	/**
	 * The last sector of the allocation table that should be accessed
	 **/
	uint global_end;
	/**
	 * The size of the buffer in multiples of the sector size
	 **/
	uint sectors;
	/**
	 * The size of one sector in byte.
	 */
	uint sector_size;
	/**
	 * The disk pointer
	 */
	struct DISK *disk;
};

/* Note: The unused warning is suppressed
for these buffers, works only for gcc */
static uint8_t __attribute__((__unused__)) AT_BUFFER[AT_BUFFER_SIZE];
static uint8_t __attribute__((__unused__)) IT_BUFFER[AT_SIZE];
static uint8_t __attribute__((__unused__)) SEC_BUFFER[SECTOR_SIZE];
static uint8_t __attribute__((__unused__)) INO_BUFFER[SECTOR_SIZE];
static struct AT_WINDOW __attribute__((__unused__)) IT_WINDOW;
static struct AT_WINDOW __attribute__((__unused__)) AT_WINDOW;

/**
 * Initializes the AT_WINDOW struct for the allocation table and
 * loads the first window
 * @param[out] win The AT_WINDOW struct to be filled
 * @param[in] fs The file system struct
 * @param[in] buffer Pointer to the buffer for the window buffer
 * */
bool init_window(struct AT_WINDOW *win, struct FILE_SYSTEM *fs,
		uint8_t *buffer);
/* Initializes the AT_WINDOW struct for the inode allocation table and
 * loads the first window.
 * @param[out] win The AT_WINDOW struct to be filled
 * @param[in] fs The file system struct
 * @param[in] buffer Pointer to the buffer for the window buffer
 * */
bool init_window_it(struct AT_WINDOW *win, struct FILE_SYSTEM *fs,
		uint8_t *buffer);
/**
 * Saves the content of the buffer and then moves the window and
 * loads the new part of the allocation table.
 * @param[in] win Pointer to the AT_WINDOW struct to be moved
 * @param[in] index The index the window should be moved to
 **/
bool move_window(struct AT_WINDOW *win, int index);

/**
 * Saves the content of the buffer to disk
 * @param[in] win Pointer to the AT_WINDOW struct that should be saved
 */
bool save_window(struct AT_WINDOW *win);

/**
 *	Reloads the buffer of the window.
 *	@param[in] win Pointer to the AT_WINDOW struct that should be reloaded
 */
bool reload_window(struct AT_WINDOW *win);

#endif /* WINDOW_BUFFER_H */
