#include "inode_functions.h"


/* Finds the first inode that can be deleted and returns that inode*/
bool find_ino_length(struct FILE_SYSTEM *fs, struct INODE *file, uint size)
{
	uint i, k, ino_cnt, *pos;
	struct INODE *inodes;

	pos = malloc(fs->inode_sec * sizeof(uint));
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, IT_BUFFER, fs->inode_sec * k, pos, &ino_cnt);
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
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);
	for (i = 0; i < size; ++i)
		ret_val += popcount(IT_BUFFER[i]);

	for (k = 0; k < (fs->inode_max % 8); ++k) {
		tmp = 0x80 >> k;
		if (tmp & IT_BUFFER[i])
			++ret_val;
	}

	return ret_val;
}

/* Returns the position of all inodes in a block */
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
void load_inodes_block(struct FILE_SYSTEM *fs, struct INODE *buffer)
{
	uint i, *ino_pos, ino_cnt, offset, ino_off;

	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	offset = 0;
	ino_off = 0;
	ino_pos = malloc(fs->inode_sec * sizeof(uint32_t));

	for (i = 0; i < fs->inode_block_size; ++i) {
		get_ino_pos(fs, IT_BUFFER, offset, ino_pos, &ino_cnt);
		load_inode_block(fs, &buffer[ino_off], ino_pos, ino_cnt, i);
		ino_off += ino_cnt;
		offset += fs->inode_sec;
	}

	free(ino_pos);
}

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