#include "window_buffer.h"


bool init_window(struct AT_WINDOW *win, struct FILE_SYSTEM *fs, uint8_t *buffer)
{
	int i;

	win->isValid = false;
	win->buffer = buffer;
	win->global_start = fs->alloc_table;
	win->global_end = fs->alloc_table + fs->alloc_table_size - 1;
	win->sectors = fs->alloc_table_buffer_size / fs->sector_size;
	/* Just in case the alloc_table is smaller than the buffer... */
	if (win->sectors > fs->alloc_table_size)
		win->sectors = fs->alloc_table_size;

	win->sector_size = fs->sector_size;
	win->disk = fs->disk;

	for (i = 0; i < fs->alloc_table_buffer_size; ++i)
		win->buffer[i] = 0x00;

	if (disk_read(win->disk, win->buffer,
			win->global_start, win->sectors) != RES_OK)
		return false;

	win->global_index = win->global_start;
	win->isValid = true;
	return true;
}

bool init_window_it(struct AT_WINDOW *win,
		struct FILE_SYSTEM *fs, uint8_t *buffer)
{
	int i;

	win->isValid = false;
	win->buffer = buffer;
	win->global_start = fs->inode_alloc_table;
	win->global_end = fs->inode_alloc_table +
			fs->inode_alloc_table_size - 1;
	win->sectors = fs->inode_alloc_table_buffer_size / fs->sector_size;
	/* Just in case the alloc_table is smaller than the buffer... */
	if (win->sectors > fs->inode_alloc_table_size)
		win->sectors = fs->inode_alloc_table_size;

	win->sector_size = fs->sector_size;
	win->disk = fs->disk;

	for (i = 0; i < fs->inode_alloc_table_buffer_size; ++i)
		win->buffer[i] = 0x00;

	if (disk_read(win->disk, win->buffer,
			win->global_start, win->sectors) != RES_OK)
		return false;

	win->global_index = win->global_start;
	win->isValid = true;
	return true;
}

bool move_window(struct AT_WINDOW *win, int index)
{
	uint new_index = win->global_start + index;
	/* Parameter checking */
	if (new_index > win->global_end || index < 0)
		return false;

	/* Check if the window is already at the right position */
	if (win->global_index == new_index)
		return true;

	/* Save and new load */
	if (disk_write(win->disk, win->buffer,
			win->global_index, win->sectors) != RES_OK)
		return false;

	win->isValid = false;
	win->global_index = new_index;
	if (disk_read(win->disk, win->buffer,
			win->global_index, win->sectors) != RES_OK)
		return false;

	win->isValid = true;
	return true;
}

bool save_window(struct AT_WINDOW *win)
{
	if (disk_write(win->disk, win->buffer,
			win->global_index, win->sectors) != RES_OK)
		return false;
	return true;
}

bool reload_window(struct AT_WINDOW *win)
{
	if (disk_read(win->disk, win->buffer,
			win->global_index, win->sectors) != RES_OK)
		return false;
	return true;
}
