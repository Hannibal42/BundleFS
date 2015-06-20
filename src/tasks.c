#include "tasks.h"

void defragment(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint8_t *buffer, *al_tab;
	uint ino_cnt, k, sec_cnt, tmp, al_tab_sec, old_loc;
	int i;

	ino_cnt = inodes_used(fs);
	inodes = malloc(inodes_used(fs) * sizeof(struct INODE));
	load_inodes(fs, inodes);

	al_tab_sec = fs->alloc_table_size * fs->sector_size;
	al_tab = malloc(al_tab_sec);
	disk_read(fs->disk, (char *) al_tab, fs->alloc_table,
		fs->alloc_table_size);

	quicksort_inodes(inodes, ino_cnt);

	k = fs->sector_count;
	for (i = ino_cnt - 1; i >= 0; --i) {
		sec_cnt = div_up(inodes[i].size, fs->sector_size);
		sec_cnt += div_up(inodes[i].check_size, fs->sector_size);

		buffer = malloc(sec_cnt * fs->sector_size);
		disk_read(fs->disk, (char *) buffer, inodes[i].location,
			sec_cnt);

		tmp = find_seq(al_tab, al_tab_sec, sec_cnt);

		if (tmp < 0) {
			k = inodes[i].location;
			free(buffer);
			continue;
		}

		disk_write(fs->disk, (char *) buffer,
			fs->sector_count - (tmp + sec_cnt), sec_cnt);
		write_seq(al_tab, tmp, sec_cnt);
		disk_write(fs->disk, (char *) al_tab, fs->alloc_table,
			fs->alloc_table_size);
		old_loc = inodes[i].location;
		inodes[i].location = fs->sector_count - (tmp + sec_cnt);
		write_inode(fs, &inodes[i]);

		if (k == (fs->sector_count - tmp)) {
			k = inodes[i].location;
			free(buffer);
			delete_seq(al_tab, fs->sector_count - old_loc - sec_cnt,
				sec_cnt);
			disk_write(fs->disk, (char *) al_tab, fs->alloc_table,
			fs->alloc_table_size);
			continue;
		}

		delete_seq(al_tab, fs->sector_count - old_loc, sec_cnt);
		write_seq(al_tab, fs->sector_count - (k - sec_cnt), sec_cnt);

		disk_write(fs->disk, (char *) buffer, k - sec_cnt, sec_cnt);
		disk_write(fs->disk, (char *) al_tab, fs->alloc_table,
			fs->alloc_table_size);
		inodes[i].location = k - sec_cnt;
		write_inode(fs, &inodes[i]);
		delete_seq(al_tab, tmp, sec_cnt);
		disk_write(fs->disk, (char *) al_tab, fs->alloc_table,
			fs->alloc_table_size);
		k = inodes[i].location;
		free(buffer);
	}
	free(inodes);
	free(al_tab);
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
