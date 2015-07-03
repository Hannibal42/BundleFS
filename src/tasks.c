#include "tasks.h"

#include "buffer.h"

void extern load_all_tab(uint8_t *buffer,struct FILE_SYSTEM *fs);
void extern load_ino_tab(uint8_t *buffer, struct FILE_SYSTEM *fs);


//TODO: Copy the data block by block
//TODO: More comments
void defragment(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint ino_cnt, k, sec_cnt, al_tab_sec, old_loc, che_size;
	int i, r, tmp;

	ino_cnt = inodes_used(fs);
	inodes = malloc(inodes_used(fs) * sizeof(struct INODE));
	load_inodes(fs, inodes);

	al_tab_sec = fs->alloc_table_size * fs->sector_size;
	disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
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
		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);

		/* Makes a copy of the file */
		for (r = 0; r < sec_cnt; ++r) {
			disk_read(fs->disk, (char *) SEC_BUFFER, inodes[i].location + r,
				1);
			disk_write(fs->disk, (char *) SEC_BUFFER,
				(fs->sector_count - (tmp + sec_cnt)) + r, 1);
		}

		old_loc = inodes[i].location;
		inodes[i].location = fs->sector_count - (tmp + sec_cnt);
		write_inode(fs, &inodes[i]);

		/* Checks if the copy is at the right place already */
		if (k == (fs->sector_count - tmp)) {
			k = inodes[i].location;
			delete_seq(AT_BUFFER, fs->sector_count - old_loc - sec_cnt,
				sec_cnt);
			disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
			continue;
		}

		delete_seq(AT_BUFFER, fs->sector_count - old_loc, sec_cnt);
		write_seq(AT_BUFFER, fs->sector_count - (k - sec_cnt), sec_cnt);

		/* Copy file to the right place */
		for (r = 0; r < sec_cnt; ++r) {
		disk_write(fs->disk, (char *) SEC_BUFFER,
				(fs->sector_count - (tmp + sec_cnt)) + r, 1);
		disk_write(fs->disk, (char *) SEC_BUFFER, k - sec_cnt + r, 1);
		}

		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		inodes[i].location = k - sec_cnt;
		write_inode(fs, &inodes[i]);
		delete_seq(AT_BUFFER, tmp, sec_cnt);
		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		k = inodes[i].location;
	}
	free(inodes);
}

/* This should be a regular task executed by freeRTOS */
void delete_invalid_inodes(struct FILE_SYSTEM *fs)
{
	uint i, in_cnt;
	struct INODE *inodes;
	uint *tmp;

	in_cnt = inodes_used(fs);
	tmp = malloc(in_cnt * sizeof(uint) * 2);
	inodes = malloc(in_cnt * sizeof(struct INODE));
	load_inodes(fs, inodes);

	for (i = 0; i < in_cnt; ++i) {
		if (inodes[i].size > 0 && isNotValid(&inodes[i])) {
			tmp[i * 2] = inodes[i].id;
			tmp[i * 2 + 1] = inodes[i].inode_offset;
			fs_delete(fs, &inodes[i]);
		}
	}

	/* TODO: Notify upcn which Bundles have expiered */
	free(tmp);
	free(inodes);
}

void restore_fs(struct FILE_SYSTEM *fs)
{
	struct INODE* inodes;
	uint ino_cnt, tmp, ino_size, i;

	ino_cnt = inodes_used(fs);
	inodes = malloc(ino_cnt * sizeof(struct INODE));
	load_inodes(fs, inodes);

	quicksort_inodes(inodes, ino_cnt);

	for (i = 0; i < fs->alloc_table_size * fs->sector_size; ++i) {
		if (i < (fs->sector_count / 8)) {
			AT_BUFFER[i] = 0x00;
		} else {
			AT_BUFFER[i] = 0xFF;
		}
	}

	tmp = fs->sector_count;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq(AT_BUFFER, tmp, fs->inode_block +
		fs->inode_block_size);

	for (i = 0; i < ino_cnt; i++) {
		ino_size = div_up(inodes[i].size, fs->sector_size - check_size());
		tmp = fs->sector_count - inodes[i].location - ino_size;
		write_seq(AT_BUFFER, tmp, ino_size);
	}

	disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	free(inodes);
}
