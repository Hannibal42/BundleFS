#include "../h/fs_test.h"


struct disk *disk1, *disk2, *disk3, *disk4;
struct FILE_SYSTEM fs1, fs2, fs3;
uint8_t *table1, *table1_empty, *table2_empty, *table2;

extern void write_bit(uint index, uint8_t *table, bool value);

extern int find_bit(uint8_t *table, uint size);

extern int find_sequence(uint8_t *table, uint table_size, uint length);
/* Writes a sequence of 1 */
extern void write_seq(uint index, uint length, uint8_t *table);
/* Lets the bits toggle */
extern void delete_seq(uint index, uint length, uint8_t *table);
extern int get_last_free_bit(uint8_t byte);
/* Returns the following bits, starting from an index */
extern int get_free_bit(uint8_t index, uint8_t byte);
extern int popcount(uint8_t byte);


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

bool get_last_free_bit_test(void)
{
	uint8_t temp;

	temp = 0x00;
	if (get_last_free_bit(temp) != 8)
		return false;
	temp = 0x80;
	if (get_last_free_bit(temp) != 7)
		return false;
	temp = 0xC0;
	if (get_last_free_bit(temp) != 6)
		return false;
	temp = 0xE0;
	if (get_last_free_bit(temp) != 5)
		return false;
	temp = 0xF0;
	if (get_last_free_bit(temp) != 4)
		return false;
	temp = 0x08;
	if (get_last_free_bit(temp) != 3)
		return false;
	temp = 0x8C;
	if (get_last_free_bit(temp) != 2)
		return false;
	temp = 0x1E;
	if (get_last_free_bit(temp) != 1)
		return false;
	temp = 0x2F;
	if (get_last_free_bit(temp) != 0)
		return false;

	return true;
}



bool get_free_bit_test(void)
{
	uint8_t temp;

	temp =  0x00;
	if (get_free_bit(0, temp) != 8)
		return false;
	if (get_free_bit(3, temp) != 5)
		return false;
	if (get_free_bit(8, temp) != -1)
		return false;

	temp = 0xFF;
	if (get_free_bit(0, temp) != 0)
		return false;
	if (get_free_bit(10, temp) != -1)
		return false;
	if (get_free_bit(3, temp) != 0)
		return false;

	temp = 0x87;
	if (get_free_bit(1, temp) != 4)
		return false;
	if (get_free_bit(2, temp) != 3)
		return false;

	temp = 0xF0;
	if (get_free_bit(3, temp) != 0)
		return false;
	if (get_free_bit(4, temp) != 4)
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
	uint i;

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

	for (i = 0; i < 256; ++i)
		table2[i] = 0x00;
	if (find_sequence(table1, 256, 256) != 0)
		return false;
	if (find_sequence(table1, 256, 257) != -1)
		return false;

	return true;
}

bool write_seq_test(void)
{
	uint i;

	write_seq(0, 7, table1_empty);
	if (table1_empty[0] != 0xFE)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(7, 1, table1_empty);
	if (table1_empty[0] != 0xFF)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(8, 15, table1_empty);
	if (table1_empty[1] != 0xFF)
		return false;
	if (table1_empty[2] != 0xFE)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1_empty[i] != 0x00)
			return false;

	write_seq(25, 10, table1_empty);
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

	delete_seq(0, 4, table1);
	if (table1[0] != 0x0F)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(4, 3, table1);
	if (table1[0] != 0x01)
		return false;
	for (i = 1; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(7, 10, table1);
	if (table1[0] != 0x00)
		return false;
	if (table1[1] != 0x00)
		return false;
	if (table1[2] != 0x7F)
		return false;
	for (i = 3; i < 64; ++i)
		if (table1[i] != 0xFF)
			return false;

	delete_seq(0, 512, table1);
	for (i = 0; i < 64; ++i)
		if (table1[i] != 0x00)
			return false;

	return true;
}

bool mkfs_test(void)
{

	return false;
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
		table2[i] = 0x00;
	for (i = 0; i < 256; ++i)
		table2[i] = 0xFF;
}



void tearDown(void)
{
	free(disk1);
	free(disk2);
	free(disk3);
	free(disk4);
	free(table1);
	free(table2);
}

bool write_bit_test(void)
{

	table1 = malloc(sizeof(uint8_t) * 5);
	table1[0] = 0xFF;
	table1[1] = 0XF8;
	table1[2] = 0x00;
	table1[3] = 0xFF;
	table1[4] = 0xFF;

	write_bit(0, table1, 0);
	if (table1[0] != 0x7F)
		return false;

	write_bit(0, table1, 1);
	if (table1[0] != 0xFF)
		return false;

	write_bit(8, table1, 1);
	if (table1[1] != 0xF8)
		return false;

	write_bit(8, table1, 0);
	if (table1[1] != 0x78)
		return false;

	write_bit(18, table1, 1);
	if (table1[2] != 0x20)
		return false;

	write_bit(39, table1, 0);
	if (table1[4] != 0xFE)
		return false;

	return true;
}


int main(void)
{
	uint i, length;

	bool (*tests[9]) (void);
	tests[0] = write_bit_test;
	tests[1] = find_bit_test;
	tests[2] = find_sequence_test;
	tests[3] = write_seq_test;
	tests[4] = mkfs_test;
	tests[5] = delete_seq_test;
	tests[6] = get_free_bit_test;
	tests[7] = get_last_free_bit_test;
	tests[8] = popcount_test;

	length = sizeof(tests) / sizeof(tests[0]);
	for (i = 0; i < length; ++i) {
		setUp();
		if (!(*tests[i])())
			printf("Error in Test %d\n", i);
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
} */
