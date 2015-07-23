#include "tasks.h"

#include "buffer.h"

/* Defragments the files on disk */
void defragment(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint ino_cnt, k, sec_cnt, al_tab_sec, old_loc, che_size;
	int i, r, tmp;

	ino_cnt = inodes_used(fs);
	inodes = malloc(ino_cnt * sizeof(struct INODE));
	load_inodes_block(fs, inodes);

	al_tab_sec = fs->alloc_table_size * fs->sector_size;
	disk_read(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	/* The Inodes are sorted by their location */
	quicksort_inodes(inodes, ino_cnt);

	k = fs->sector_count;
	for (i = ino_cnt - 1; i >= 0; --i) {
		che_size = fs->sector_size - check_size();
		sec_cnt = div_up(inodes[i].size, che_size);

		tmp = find_seq(AT_BUFFER, al_tab_sec, sec_cnt);

		/* Checks if the file can be copied */
		if (tmp < 0) {
			k = inodes[i].location;
			continue;
		}

		write_seq(AT_BUFFER, tmp, sec_cnt);
		disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);

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
			delete_seq(AT_BUFFER,
				fs->sector_count - old_loc - sec_cnt, sec_cnt);
			disk_write(fs->disk, AT_BUFFER,
				fs->alloc_table, fs->alloc_table_size);
			continue;
		}

		delete_seq(AT_BUFFER, fs->sector_count - old_loc, sec_cnt);
		write_seq(AT_BUFFER, fs->sector_count - (k - sec_cnt), sec_cnt);

		/* Copy file to the right place */
		for (r = 0; r < sec_cnt; ++r) {
			disk_write(fs->disk, SEC_BUFFER,
				(fs->sector_count - (tmp + sec_cnt)) + r, 1);
			disk_write(fs->disk, SEC_BUFFER,
				k - sec_cnt + r, 1);
		}

		disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		inodes[i].location = k - sec_cnt;
		write_inode(fs, &inodes[i]);
		delete_seq(AT_BUFFER, tmp, sec_cnt);
		disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		k = inodes[i].location;
	}
	free(inodes);
}

/* This should be a regular task executed by upcn */
void delete_invalid_inodes(struct FILE_SYSTEM *fs)
{
	uint i, k, ino_cnt, *pos;
	struct INODE *inodes;
	uint *tmp;

	/* TODO: Maybe make tmp into a dynamic data structure */
	tmp = malloc(inodes_used(fs) * sizeof(uint));
	pos = malloc(fs->inode_sec * sizeof(uint));
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, IT_BUFFER, fs->inode_sec * k, pos, &ino_cnt);
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
	uint ino_cnt, tmp, ino_size, i, k, *pos;


	/* Wipes the allocation table clean */
	tmp = (fs->sector_count / 8);
	for (i = 0; i < fs->alloc_table_size * fs->sector_size; ++i) {
		if (i < tmp)
			AT_BUFFER[i] = 0x00;
		else
			AT_BUFFER[i] = 0xFF;
	}

	/* Write the superblock into the allocation table */
	tmp = fs->sector_count;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq(AT_BUFFER, tmp, fs->inode_block + fs->inode_block_size);

	/* Loads every valid inode and write it into the allocation table */
	pos = malloc(fs->inode_sec * sizeof(uint));
	disk_read(fs->disk, IT_BUFFER, fs->inode_alloc_table,
		fs->inode_alloc_table_size);

	for (k = 0; k < fs->inode_block_size; ++k) {
		get_ino_pos(fs, IT_BUFFER, fs->inode_sec * k, pos, &ino_cnt);
		inodes = (struct INODE *) INO_BUFFER;
		load_inode_block(fs, inodes, pos, ino_cnt, k);
		for (i = 0; i < ino_cnt; ++i) {
			ino_size = div_up(inodes[i].size,
				fs->sector_size - check_size());
			tmp = fs->sector_count - inodes[i].location - ino_size;
			write_seq(AT_BUFFER, tmp, ino_size);
		}
	}

	disk_write(fs->disk, AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	free(pos);
}
