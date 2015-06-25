#include "tasks.h"

#include "buffer.h"

extern unsigned long div_up(unsigned long dividend,
	unsigned long divisor);


//TODO: Copy the data block by block
void defragment(struct FILE_SYSTEM *fs)
{
	struct INODE *inodes;
	uint8_t *buffer;
	uint ino_cnt, k, sec_cnt, tmp, al_tab_sec, old_loc;
	int i;

	ino_cnt = inodes_used(fs);
	inodes = malloc(inodes_used(fs) * sizeof(struct INODE));
	load_inodes(fs, inodes);

	al_tab_sec = fs->alloc_table_size * fs->sector_size;
	disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	quicksort_inodes(inodes, ino_cnt);

	k = fs->sector_count;
	for (i = ino_cnt - 1; i >= 0; --i) {
		sec_cnt = div_up(inodes[i].size, fs->sector_size);
		//TODO: Align to the new check size

		buffer = malloc(sec_cnt * fs->sector_size);
		disk_read(fs->disk, (char *) buffer, inodes[i].location,
			sec_cnt);

		tmp = find_seq(AT_BUFFER, al_tab_sec, sec_cnt);

		if (tmp < 0) {
			k = inodes[i].location;
			free(buffer);
			continue;
		}

		disk_write(fs->disk, (char *) buffer,
			fs->sector_count - (tmp + sec_cnt), sec_cnt);
		write_seq(AT_BUFFER, tmp, sec_cnt);
		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		old_loc = inodes[i].location;
		inodes[i].location = fs->sector_count - (tmp + sec_cnt);
		write_inode(fs, &inodes[i]);

		if (k == (fs->sector_count - tmp)) {
			k = inodes[i].location;
			free(buffer);
			delete_seq(AT_BUFFER, fs->sector_count - old_loc - sec_cnt,
				sec_cnt);
			disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
			continue;
		}

		delete_seq(AT_BUFFER, fs->sector_count - old_loc, sec_cnt);
		write_seq(AT_BUFFER, fs->sector_count - (k - sec_cnt), sec_cnt);

		disk_write(fs->disk, (char *) buffer, k - sec_cnt, sec_cnt);
		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		inodes[i].location = k - sec_cnt;
		write_inode(fs, &inodes[i]);
		delete_seq(AT_BUFFER, tmp, sec_cnt);
		disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
			fs->alloc_table_size);
		k = inodes[i].location;
		free(buffer);
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

	disk_read(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	tmp = fs->sector_count;
	tmp -= (fs->inode_block + fs->inode_block_size);
	write_seq(AT_BUFFER, tmp, fs->inode_block +
		fs->inode_block_size);

	for (i = 0; i < ino_cnt; i++) {
		ino_size = div_up(inodes[i].size, fs->sector_size);
		//TODO: The calculation needs to be alligned to the new check size
		tmp = fs->sector_count - inodes[i].location - ino_size;
		write_seq(AT_BUFFER, tmp, ino_size);
	}

	disk_write(fs->disk, (char *) AT_BUFFER, fs->alloc_table,
		fs->alloc_table_size);

	free(inodes);
}
