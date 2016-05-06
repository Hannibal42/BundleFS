#include "maintenance_tasks.h"

/* Defragments the files on disk */
void defragment(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint ino_cnt, k, sec_cnt, old_loc, che_size, tmp;
	int i, r;

	ino_cnt = inodes_used(fs);
	inodes = malloc(ino_cnt * sizeof(struct INODE));
	load_all_inodes(fs, inodes);

	/* The Inodes are sorted by their location */
	quicksort_inodes(inodes, ino_cnt);

	k = fs->sector_count;
	for (i = ino_cnt - 1; i >= 0; --i) {
		che_size = fs->sector_size - check_size();
		sec_cnt = div_up(inodes[i].size, che_size);

		/* Checks if the file can be copied */
		if (!find_seq_global(fs->at_win, sec_cnt, &tmp)) {
			k = inodes[i].location;
			continue;
		}

		write_seq_global(fs->at_win, tmp, sec_cnt);
		if (!save_window(fs->at_win))
			return;

		/* Makes a copy of the file block by block*/
		for (r = 0; r < sec_cnt; ++r) {
			disk_read(fs->disk, SEC_BUFFER,
				inodes[i].location + r, 1);
			disk_write(fs->disk, SEC_BUFFER,
				(fs->sector_count - (tmp + sec_cnt)) + r, 1);
		}

		old_loc = inodes[i].location;
		inodes[i].location = fs->sector_count - (tmp + sec_cnt);
		write_inode(fs, &inodes[i]);

		/* Checks if the copy is at the right place already */
		if (k == (fs->sector_count - tmp)) {
			k = inodes[i].location;
			if (!delete_seq_global(fs->at_win,
					fs->sector_count - old_loc -
					sec_cnt, sec_cnt))
				return;
			if (!save_window(fs->at_win))
				return;
			continue;
		}

		if (!delete_seq_global(fs->at_win,
				fs->sector_count - old_loc, sec_cnt))
			return;
		if (!write_seq_global(fs->at_win,
				fs->sector_count - (k - sec_cnt), sec_cnt))
			return;

		/* Copy file to the right place */
		for (r = 0; r < sec_cnt; ++r) {
			disk_write(fs->disk, SEC_BUFFER,
				(fs->sector_count - (tmp + sec_cnt)) + r, 1);
			disk_write(fs->disk, SEC_BUFFER,
				k - sec_cnt + r, 1);
		}

		if (!save_window(fs->at_win))
			return;
		inodes[i].location = k - sec_cnt;
		write_inode(fs, &inodes[i]);
		if (!delete_seq_global(fs->at_win, tmp, sec_cnt))
			return;
		if (!save_window(fs->at_win))
			return;
		k = inodes[i].location;
	}
	free(inodes);
}

void delete_invalid_inodes(struct FILE_SYSTEM *fs)
{
	uint i, k, ino_cnt, *pos;
	struct INODE *inodes;
	uint *tmp;

	/* TODO: Maybe make tmp into a dynamic data structure */
	tmp = malloc(inodes_used(fs) * sizeof(uint));
	pos = malloc(fs->inode_sec * sizeof(uint));

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, fs->inode_sec * k, pos, &ino_cnt);
		inodes = (struct INODE *) INO_BUFFER;
		load_inode_block(fs, inodes, pos, ino_cnt, k);
		for (i = 0; i < ino_cnt; ++i) {
			if (isNotValid(&inodes[i])) {
				tmp[k] = inodes[i].inode_offset;
				fs_delete(fs, &inodes[i]);
			}
		}
	}

	/* TODO: Notify upcn which Bundles have expiered */
	free(tmp);
	free(pos);
}


/* This function writes the allocation table new, this is used to fix
blocks that are marked allocated, but dont belong to an inode */
void restore_fs(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint ino_cnt, tmp, ino_size, i, k, m, *pos;

	/* Wipes the allocation table clean */
	tmp = (fs->sector_count / 8);
	for (i = 0; i < fs->alloc_table_size; ++i) {
		m = i * fs->alloc_table_buffer_size;
		for (k = 0; k < fs->alloc_table_buffer_size; ++k) {
			if (m + k < tmp)
				fs->at_win->buffer[k] = 0x00;
			else
				fs->at_win->buffer[k] = 0xFF;
		}
	}

	/* Write the superblock into the allocation table */
	tmp = fs->sector_count;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq_global(fs->at_win, tmp,
			fs->inode_block + fs->inode_block_size);

	/* Loads every valid inode and write it into the allocation table */
	pos = malloc(fs->inode_sec * sizeof(uint));

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, fs->inode_sec * k, pos, &ino_cnt);
		inodes = (struct INODE *) INO_BUFFER;
		load_inode_block(fs, inodes, pos, ino_cnt, k);
		for (i = 0; i < ino_cnt; ++i) {
			ino_size = div_up(inodes[i].size,
				fs->sector_size - check_size());
			tmp = fs->sector_count - inodes[i].location - ino_size;
			write_seq_global(fs->at_win, tmp, ino_size);
		}
	}

	save_window(fs->at_win);

	free(pos);
}
