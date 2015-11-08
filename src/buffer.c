#include "buffer.h"


bool init_window(struct AT_WINDOW *win, struct FILE_SYSTEM *fs, uint8_t *buffer)
{
	win->isValid = false;
	win->buffer = buffer;
	win->global_start = fs->alloc_table;
	win->global_end = fs->alloc_table + fs->alloc_table_size;
	win->sectors = fs->alloc_table_buffer_size;
	win->disk = fs->disk;

	if (disk_read(win->disk, win->buffer, win->global_start, win->sectors) != RES_OK)
		return false;

	win->global_index = win->global_start;
	win->isValid = true;
	return true;
}

bool move_window(struct AT_WINDOW *win, uint8_t index)
{
	/* Parameter checking */
	if (index < win->global_start)
		return false;
	if (index + win->sectors > win->global_end)
		return false;

	/* Save and new load */
	if (disk_write(win->disk, win->buffer, win->global_index, win->sectors) != RES_OK)
		return false;

	win->isValid = false;
	win->global_index = win->global_start + index;
	if (disk_read(win->disk, win->buffer, win->global_index, win->sectors) != RES_OK)
		return false;

	win->isValid = true;
	return true;
}

bool save_window(struct AT_WINDOW *win)
{
	if (disk_write(win->disk, win->buffer, win->global_index, win->sectors) != RES_OK)
		return false;
	return true;
}