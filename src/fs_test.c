#include "../h/fs_test.h"


struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
uint8_t *table1, *table1_empty, *table2_empty, *table2;
uint8_t *data1, *data2, *data3;

extern void write_bit(uint8_t *table, uint index, bool value);

extern int find_bit(const uint8_t *table, uint size);

extern int find_sequence(const uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
extern void write_seq(uint8_t *table, uint index, uint length);
/* Lets the bits toggle */
extern void delete_seq(uint8_t *table, uint index, uint length);
extern int last_free_bits(uint8_t byte);
/* Returns the following bits, starting from an index */
extern int get_free_bit(uint8_t index, uint8_t byte);
extern int popcount(uint8_t byte);
extern int find_sequence_small(const uint8_t *table, uint table_size,
	uint length);
extern bool checksum_check(const uint8_t *buffer, const struct INODE *file,
uint sector_size);
extern void checksum(const uint8_t *buffer, uint lenght, uint8_t *result,
	uint size);
extern unsigned long div_up(unsigned long dividend,
	unsigned long divisor);

bool fs_mount_test(void)
{
	uint i, tmp;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};

	for (i = 0; i < 4; ++i) {
		disk_initialize(disks[i]);
		fs_mount(disks[i], &fs1);

		if (fs1.sector_size != disks[i]->sector_size)
			return false;
		if (fs1.sector_count != disks[i]->sector_count)
			return false;
		tmp = div_up(sizeof(struct FILE_SYSTEM), fs1.sector_size);
		if (fs1.alloc_table != tmp)
			return false;
		tmp = div_up(fs1.sector_count, fs1.sector_size * 8);
		if (fs1.alloc_table_size != tmp)
			return false;
		tmp += fs1.alloc_table;
		if (fs1.inode_alloc_table != tmp)
			return false;
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		tmp = div_up(tmp, fs1.sector_size);
		if (fs1.inode_alloc_table_size != tmp)
			return false;
		tmp += fs1.inode_alloc_table;
		if (fs1.inode_block != tmp)
			return false;
		tmp = fs1.sector_count / 8;
		if (tmp < 8)
			tmp = 8;
		if (fs1.inode_block_size != tmp)
			return false;
	}

	return true;
}


bool checksum_check_test(void)
{
	uint i;
	struct INODE file;
	uint8_t *check, *tmp;

	check = malloc(64);
	tmp = malloc(192);

	file.size = 128;
	file.check_size = 64;
	checksum(data1, 128, check, 64);

	for (i = 0; i < 128; ++i)
		tmp[i] = data1[i];

	for (i = 0; i < 64; ++i)
		tmp[i + 128] = check[i];

	if (!checksum_check(tmp, &file, 64))
		return false;

	file.size = 64;
	file.check_size = 16;
	if (checksum_check(data1, &file, 16))
		return false;

	free(check);
	free(tmp);
	return true;
}

bool find_sequence_small_test(void)
{

	if (find_sequence_small(table1, 64, 10) != -1)
		return false;
	table1[0] = 0x87;
	if (find_sequence_small(table1, 64, 4) != 1)
		return false;
	table1[1] = 0x08;
	table1[2] = 0x0F;
	if (find_sequence_small(table1, 64, 5) != 13)
		return false;
	if (find_sequence_small(table1, 64, 6) != 13)
		return false;
	table1[3] = 0x00;
	table1[4] = 0x01;
	if (find_sequence_small(table1, 64, 8) != 24)
		return false;
	if (find_sequence_small(table1, 64, 8) != 24)
		return false;
	if (find_sequence_small(table1, 64, 1) != 1)
		return false;

	if (find_sequence_small(table2, 256, 10) != -1)
		return false;
	table2[0] = 0x40;
	table2[1] = 0x00;
	table2[2] = 0x00;
	table2[3] = 0x00;
	table2[4] = 0x00;
	if (find_sequence_small(table2, 256, 5) != 2)
		return false;

	return true;
}

bool popcount_test(void)
{
	uint k, count;
	uint8_t i, tmp;

	for (i = 0; i < 255; ++i) {
		count = 0;
		tmp = i;
		for (k = 128; k > 0; k /= 2) {
			if (tmp / k) {
				count += 1;
				tmp -= k;
			}
		}
		if (popcount(i) != count)
			return false;
	}
	return true;
}

bool last_free_bits_test(void)
{
	uint8_t tmp;

	tmp = 0x00;
	if (last_free_bits(tmp) != 8)
		return false;
	tmp = 0x80;
	if (last_free_bits(tmp) != 7)
		return false;
	tmp = 0xC0;
	if (last_free_bits(tmp) != 6)
		return false;
	tmp = 0xE0;
	if (last_free_bits(tmp) != 5)
		return false;
	tmp = 0xF0;
	if (last_free_bits(tmp) != 4)
		return false;
	tmp = 0x08;
	if (last_free_bits(tmp) != 3)
		return false;
	tmp = 0x8C;
	if (last_free_bits(tmp) != 2)
		return false;
	tmp = 0x1E;
	if (last_free_bits(tmp) != 1)
		return false;
	tmp = 0x2F;
	if (last_free_bits(tmp) != 0)
		return false;

	return true;
}

bool get_free_bit_test(void)
{
	uint8_t tmp;

	tmp =  0x00;
	if (get_free_bit(0, tmp) != 8)
		return false;
	if (get_free_bit(3, tmp) != 5)
		return false;
	if (get_free_bit(8, tmp) != -1)
		return false;

	tmp = 0xFF;
	if (get_free_bit(0, tmp) != 0)
		return false;
	if (get_free_bit(10, tmp) != -1)
		return false;
	if (get_free_bit(3, tmp) != 0)
		return false;

	tmp = 0x87;
	if (get_free_bit(1, tmp) != 4)
		return false;
	if (get_free_bit(2, tmp) != 3)
		return false;

	tmp = 0xF0;
	if (get_free_bit(3, tmp) != 0)
		return false;
	if (get_free_bit(4, tmp) != 4)
		return false;

	return true;
}

bool find_bit_test(void)
{
	table1[10] = 0x7F;
	if (find_bit(table1, 64) != 80)
		return false;

	table1[10] = 0x00;
	if (find_bit(table1, 64) != 80)
		return false;

	table1[9] = 0xF7;
	if (find_bit(table1, 64) != 76)
		return false;

	table2[9] = 0xF7;
	if (find_bit(table2, 256) != 76)
		return false;

	return true;
}
/* TODO: More cases */
bool find_sequence_test(void)
{

	table1[10] = 0x78;
	if (find_sequence(table1, 64, 1) != 80)
		return false;
	if (find_sequence(table1, 64, 2) != 85)
		return false;
	if (find_sequence(table1, 64, 3) != 85)
		return false;
	if (find_sequence(table1, 64, 4) != -1)
		return false;

	table1[11] = 0x7F;
	if (find_sequence(table1, 64, 4) != 85)
		return false;

	table1[4] = 0xF0;
	table1[5] = 0x00;
	table1[6] = 0x0F;
	if (find_sequence(table1, 64, 14) != 36)
		return false;
	if (find_sequence(table1, 64, 15) != 36)
		return false;
	if (find_sequence(table1, 64, 17) != -1)
		return false;

	if (find_sequence(table2_empty, 256, 256) != 0)
		return false;
	if (find_sequence(table2_empty, 256, 50000) != -1)
		return false;

	return true;
}

bool write_seq_test(void)
{
	uint i;

	write_seq(table1_empty, 0, 7);
	if (table1_empty[0] != 0xFE)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 7, 1);
	if (table1_empty[0] != 0xFF)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 8, 15);
	if (table1_empty[1] != 0xFF)
		return false;
	if (table1_empty[2] != 0xFE)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(table1_empty, 25, 10);
	if (table1_empty[0] != 0xFF)
		return false;
	if (table1_empty[1] != 0xFF)
		return false;
	if (table1_empty[2] != 0xFE)
		return false;
	if (table1_empty[3] != 0x7F)
		return false;
	if (table1_empty[4] != 0xE0)
		return false;
	for (i = 5; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	return true;
}

bool delete_seq_test(void)
{
	uint i;

	delete_seq(table1, 0, 4);
	if (table1[0] != 0x0F)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 4, 3);
	if (table1[0] != 0x01)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 7, 10);
	if (table1[0] != 0x00)
		return false;
	if (table1[1] != 0x00)
		return false;
	if (table1[2] != 0x7F)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(table1, 0, 512);
	for (i = 0; i < 64; ++i)
		if (table1[i] != 0x00)
			return false;

	return true;
}

bool write_bit_test(void)
{

	table1 = malloc(sizeof(uint8_t) * 5);
	table1[0] = 0xFF;
	table1[1] = 0XF8;
	table1[2] = 0x00;
	table1[3] = 0xFF;
	table1[4] = 0xFF;

	write_bit(table1, 0, 0);
	if (table1[0] != 0x7F)
		return false;

	write_bit(table1, 0, 1);
	if (table1[0] != 0xFF)
		return false;

	write_bit(table1, 8, 1);
	if (table1[1] != 0xF8)
		return false;

	write_bit(table1, 8, 0);
	if (table1[1] != 0x78)
		return false;

	write_bit(table1, 18, 1);
	if (table1[2] != 0x20)
		return false;

	write_bit(table1, 39, 0);
	if (table1[4] != 0xFE)
		return false;

	return true;
}

bool mkfs_test(void)
{
	uint k, at_size, it_size, ib_size;
	uint8_t *buffer;
	struct FILE_SYSTEM *fs;
	struct disk *disks[4] = {disk1, disk2, disk3, disk4};


	for (k = 0; k < 4; ++k) {

		buffer = malloc(sizeof(uint8_t) * disks[k]->sector_size);
		disk_initialize(disks[k]);
		fs_mkfs(disks[k]);
		at_size = div_up(disks[k]->sector_count,
			disks[k]->sector_size);
		ib_size = disks[k]->sector_count / 8;
		if (ib_size < 8)
			ib_size = 8;
		it_size = div_up(ib_size, disks[k]->sector_size);

		disk_read(disks[k], (char *) buffer, 0, 1);

		fs = (struct FILE_SYSTEM *) buffer;
		if (fs->sector_size != disks[k]->sector_size)
			return false;
		if (fs->sector_count != disks[k]->sector_count)
			return false;
		if (fs->alloc_table != 1)
			return false;
		if (fs->alloc_table_size != at_size)
			return false;
		if (fs->inode_alloc_table != (at_size + 1))
			return false;
		if (fs->inode_alloc_table_size != it_size)
			return false;
		if (fs->inode_block != (at_size + it_size + 1))
			return false;
		if (fs->inode_block_size != ib_size)
			return false;
		fs = NULL;

		/* TODO:
		disk_read(disks[k], (char *) buffer, 1, at_size);
		for (i = 0; i < (disks[k]->sector_count/8); ++i)
			if (buffer[i] != 0x00)
				return false;
		for (i = 2; i < disks[k]->sector_size; ++i)
			if (buffer[i] != 0xFF)
				return false; */

		disk_shutdown(disks[k]);
		free(buffer);
	}
	return true;
}

void setUp(void)
{
	uint i;

	/*Struct setup*/
	disk1 = disk_fill("disks/disk1.disk", 1024, 64);
	disk2 = disk_fill("disks/disk2.disk", 2048, 128);
	disk3 = disk_fill("disks/disk3.disk", 4096, 256);
	disk4 = disk_fill("disks/disk4.disk", 65536, 512);

	/*File setup*/
	disk_create(disk1, 1024);
	disk_create(disk2, 2048);
	disk_create(disk3, 4096);
	disk_create(disk4, 65536);

	/*Opens the file*/
	disk_initialize(disk1);
	disk_initialize(disk2);
	disk_initialize(disk3);
	disk_initialize(disk4);

	/*Make disk structures*/
	fs_mkfs(disk1);
	fs_mkfs(disk2);
	fs_mkfs(disk3);
	fs_mkfs(disk4);

	/*Force the fs to write*/
	disk_shutdown(disk1);
	disk_shutdown(disk2);
	disk_shutdown(disk3);
	disk_shutdown(disk4);

	/* Allocation table */
	table1 = malloc(64);
	table1_empty = malloc(64);
	table2 = malloc(256);
	table2_empty = malloc(256);

	for (i = 0; i < 64; ++i)
		table1[i] = 0xFF;
	for (i = 0; i < 64; ++i)
		table1_empty[i] = 0x00;
	for (i = 0; i < 256; ++i)
		table2_empty[i] = 0x00;
	for (i = 0; i < 256; ++i)
		table2[i] = 0xFF;

	/* Test Data */
	data1 = malloc(128);
	data2 = malloc(1024);
	data3 = malloc(2048);

	for (i = 0; i < 128; ++i)
		data1[i] = 0x0F;
	for (i = 0; i < 1024; ++i)
		data2[i] = 0xD0;
	for (i = 0; i < 2048; ++i)
		data3[i] = 0xEE;
}

void tearDown(void)
{
	free(disk1);
	free(disk2);
	free(disk3);
	free(disk4);
	free(table1);
	free(table2);
	free(table1_empty);
	free(table2_empty);
	free(data1);
	free(data2);
	free(data3);
}

int main(void)
{
	uint i, length;

	bool (*tests[12]) (void);
	tests[0] = write_bit_test;
	tests[1] = find_bit_test;
	tests[2] = find_sequence_test;
	tests[3] = write_seq_test;
	tests[4] = mkfs_test;
	tests[5] = delete_seq_test;
	tests[6] = get_free_bit_test;
	tests[7] = last_free_bits_test;
	tests[8] = popcount_test;
	tests[9] = find_sequence_small_test;
	tests[10] = checksum_check_test;
	tests[11] = fs_mount_test;

	length = sizeof(tests) / sizeof(tests[0]);
	for (i = 0; i < length; ++i) {
		setUp();
		if (!(*tests[i])())
			printf("Error in Test %d\n", i);
		else
			printf("Test %d succeeded\n", i);
		tearDown();
	}
	return 0;
}

/*
int main(void)
{
	int i;
	char *buffer;
	struct disk *disk, *disk2;
	struct INODE *inode1, *inode2, *inode3, *inode4;
	struct FILE_SYSTEM *fs;

	disk = disk_fill("test.disk", 1024, 64);
	disk_create(disk, 1024);
	disk_initialize(disk);
	print_disk(disk);

	fs_mkfs(disk);
	disk_shutdown(disk);
	disk2 = disk_fill("test.disk", 1024, 64);
	disk_initialize(disk2);

	fs = malloc(sizeof(struct FILE_SYSTEM));
	fs->disk = disk;
	fs_mount(disk2, fs);
	fs->disk = disk2;

	inode1 = (struct INODE *) malloc(sizeof(struct INODE));
	inode2 = (struct INODE *) malloc(sizeof(struct INODE));
	inode3 = (struct INODE *) malloc(sizeof(struct INODE));
	inode4 = (struct INODE *) malloc(sizeof(struct INODE));
	inode1->id = 0;
	inode2->id = 1;
	inode3->id = 2;
	inode4->id = 3;

	fs_create(fs, inode1, 30, 1000, 1);
	fs_create(fs, inode2, 30, 1000, 1);
	fs_create(fs, inode3, 80, 1000, 1);
	fs_create(fs, inode4, 30, 1000, 1);

	free(inode1);

	inode1 = NULL;

	inode1 = (struct INODE *) malloc(sizeof(struct INODE));
	fs_open(fs, 0, inode1);

	buffer = malloc(fs->sector_size * 2);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x0F;
	fs_write(fs, inode1, buffer);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x11;
	fs_write(fs, inode2, buffer);

	for (i = 0; i < (fs->sector_size * 2); ++i)
		buffer[i] = 0xFF;
	fs_write(fs, inode3, buffer);

	for (i = 0; i < fs->sector_size; ++i)
		buffer[i] = 0x01;
	fs_write(fs, inode4, buffer);


	print_inode(inode1);
	print_inode(inode2);
	inode2 = (struct INODE *) malloc(sizeof(struct INODE));
	fs_open(fs, 1, inode2);
	print_inode(inode2);
	fs_delete(fs, inode1);

	free(inode1);
	free(inode2);
	free(inode3);
	free(inode4);

	print_fs(fs);
	printf("Free: %lu\n", fs_getfree(disk2, fs));

	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0;
}*/
