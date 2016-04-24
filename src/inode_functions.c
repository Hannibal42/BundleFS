#include "inode_functions.h"


/* Finds the first inode that can be deleted and returns that inode */
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size)
{
	uint i, k, ino_cnt, *pos;
	struct INODE *inodes;

	pos = malloc(fs->inode_sec * sizeof(uint));

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos_new(fs, fs->inode_sec * k, pos, &ino_cnt);
		inodes = (struct INODE *) INO_BUFFER;
		load_inode_block(fs, inodes, pos, ino_cnt, k);
		for (i = 0; i < ino_cnt; ++i) {
			if (inodes[i].size >= size) {
				if ((!inodes[i].custody) ||
					isNotValid(&inodes[i])) {
					memcpy(file, &inodes[i],
						sizeof(struct INODE));
					free(pos);
					return true;
				}
			}
		}
	}
	free(pos);
	return false;
}

bool isNotValid(struct INODE *inode)
{
		return get_time() > inode->time_to_live;
}

/* Writes a single inode onto the disk */
void write_inode(struct FILE_SYSTEM *fs, struct INODE *file)
{
	struct INODE *tmp;
	uint sec_off, ino_off;

	sec_off = file->inode_offset;
	ino_off = sec_off % fs->inode_sec;
	sec_off /= fs->inode_sec;
	sec_off += fs->inode_block;

	disk_read(fs->disk, SEC_BUFFER, sec_off, 1);

	tmp = (struct INODE *) SEC_BUFFER;
	memcpy(&tmp[ino_off], file, sizeof(struct INODE));

	disk_write(fs->disk, SEC_BUFFER, sec_off, 1);
}

/* Returns the number of inodes that are valid */
uint inodes_used(struct FILE_SYSTEM *fs)
{
	uint i, k, size, ret_val;
	uint8_t tmp;

	size = fs->inode_max / 8;

	ret_val = 0;
	for (k = 0; k < fs->alloc_table_size; ++k) {
		move_window(fs->it_win, k);
		for (i = 0; i < size; ++i)
			ret_val += popcount(fs->it_win->buffer[i]);
	}

	for (k = 0; k < (fs->inode_max % 8); ++k) {
		tmp = 0x80 >> k;
		if (tmp & fs->it_win->buffer[i])
			++ret_val;
	}

	return ret_val;
}

/* Returns the position of all inodes in a sector */
void get_ino_pos(struct FILE_SYSTEM *fs, uint8_t *in_tab,
	uint offset, uint *pos, uint *ino_cnt)
{
	uint i, tmp_byte, shift, start;

	start = offset / 8;
	*ino_cnt = 0;
	shift = offset % 8;

	for (i = 0; i < fs->inode_sec; ++i) {
		tmp_byte = 0x80 >> shift;
		if (in_tab[start] & tmp_byte) {
			pos[*ino_cnt] = i;
			(*ino_cnt)++;
		}
		++shift;
		if (shift > 7) {
			shift = 0;
			++start;
		}
	}
}

/* Small helper function to access the allocation-table without changing the code  */
inline uint8_t access_table(struct AT_WINDOW *at_win, uint32_t position){
	uint global_index, tmp, local_index;

	tmp = at_win->sectors * at_win->sector_size;
	global_index = position / tmp;
	local_index = position % tmp;

	move_window(at_win, global_index);

	return at_win->buffer[local_index];
}

/* Returns the position of all inodes in a block */
void get_ino_pos_new(struct FILE_SYSTEM *fs,
	uint offset, uint *pos, uint *ino_cnt)
{
	uint i, tmp_byte, shift, start;

	start = offset / 8;
	*ino_cnt = 0;
	shift = offset % 8;

	for (i = 0; i < fs->inode_sec; ++i) {
		tmp_byte = 0x80 >> shift;
		if (access_table(fs->it_win, start) & tmp_byte) {
			pos[*ino_cnt] = i;
			(*ino_cnt)++;
		}
		++shift;
		if (shift > 7) {
			shift = 0;
			++start;
		}
	}
}

/* Loads all valid inodes from a block, the functions needs the position
 of all valid inodes and in ino_cnt the number of valid inodes*/
void load_inode_block(struct FILE_SYSTEM *fs, struct INODE *buffer,
	uint *pos, uint ino_cnt, uint sec_num)
{
	uint k;
	struct INODE *tmp;

	disk_read(fs->disk, SEC_BUFFER, fs->inode_block + sec_num, 1);

	tmp = (struct INODE *) SEC_BUFFER;
	for (k = 0; k < ino_cnt; ++k)
		buffer[k] = tmp[pos[k]];

}

/* Loads all inodes into the buffer, the inodes are loaded block by block */
void load_all_inodes(struct FILE_SYSTEM *fs, struct INODE *buffer)
{
	uint i, *ino_pos, ino_cnt, offset, ino_off;

	offset = 0;
	ino_off = 0;
	ino_pos = malloc(fs->inode_sec * sizeof(uint32_t));

	for (i = 0; i < fs->inode_block_size; ++i) {
		get_ino_pos_new(fs, offset, ino_pos, &ino_cnt);
		load_inode_block(fs, &buffer[ino_off], ino_pos, ino_cnt, i);
		ino_off += ino_cnt;
		offset += fs->inode_sec;
	}

	free(ino_pos);
}

/* compares to inodes positions */
int cmp_INODES(const void *a, const void *b)
{
	struct INODE *tmp_a, *tmp_b;
	int ret_val;

	tmp_a = (struct INODE *) a;
	tmp_b = (struct INODE *) b;
	ret_val = ((int)tmp_a->location - (int) tmp_b->location);
	return ret_val;
}

/* TODO: Does the qsort copy the list? */
void quicksort_inodes(struct INODE *inodes, int nitems)
{
	qsort(inodes, nitems, sizeof(struct INODE), cmp_INODES);
}
