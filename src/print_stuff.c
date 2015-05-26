#include "../h/print_stuff.h"

extern int first_free_bits(uint8_t byte);
extern int last_free_bits(uint8_t byte);

void print_fs(struct FILE_SYSTEM *fs)
{
	printf("-----------------------File System-----------------------\n");
	printf("alloc_table:%d\n", fs->alloc_table);
	printf("alloc_table_size:%d\n", fs->alloc_table_size);
	printf("inode_Alloc_table:%d\n", fs->inode_alloc_table);
	printf("inode_Alloc_table_size:%d\n", fs->inode_alloc_table_size);
	printf("inode_block:%d\n", fs->inode_block);
	printf("inode_block_size:%d\n", fs->inode_block_size);
}

void print_disk(struct disk *disk)
{
	printf("-----------------------Disk-----------------------\n");
	printf("number:%c\n", disk->number);
	printf("size:%lu\n", disk->size);
	printf("sector_size:%d\n", disk->sector_size);
	printf("sector_count:%d\n", disk->sector_count);
	switch (disk->status) {
	case STA_NOINIT:
		printf("status: STA_NOINIT");
		break;
	case STA_NODISK:
		printf("status: STA_NODISK");
		break;
	case STA_PROTECT:
		printf("status: STA_PROTECT");
		break;
	case STA_READY:
		printf("status: STA_READY");
		break;
	case STA_ERROR_NO_FILE:
		printf("status: STA_ERROR_NO_FILE");
		break;
	default:
		printf("status: ERROR");
	}
	printf("\n");
}


void print_inode(struct INODE *file)
{
	printf("-----------------------Inode-----------------------\n");
	printf("ID: %d\n", file->id);
	printf("size: %d\n", file->size);
	printf("creation date: %d\n", file->creation_date);
	printf("last modified: %d\n", file->last_modified);
	printf("offset: %d\n", file->offset);
	printf("location: %d\n", file->location);
	printf("inode offset: %d\n", file->inode_offset);
	printf("custody: %hu\n", file->custody);
	printf("TTL: %d\n", file->time_to_live);
}


void print_get_free_bit(void)
{
	printf("---------------first_free_bits -------------------\n");
	printf("Number: %d\n", first_free_bits(0xFF));
	printf("Number: %d\n", first_free_bits(0x7F));
	printf("Number: %d\n", first_free_bits(0x3F));
	printf("Number: %d\n", first_free_bits(0x1F));
	printf("Number: %d\n", first_free_bits(0x0F));
	printf("Number: %d\n", first_free_bits(0x07));
	printf("Number: %d\n", first_free_bits(0x03));
	printf("Number: %d\n", first_free_bits(0x01));
	printf("Number: %d\n", first_free_bits(0x00));

	printf("---------------last_free_bits -------------------\n");
	printf("Number: %d\n", last_free_bits(0xFF));
	printf("Number: %d\n", last_free_bits(0xFE));
	printf("Number: %d\n", last_free_bits(0xFC));
	printf("Number: %d\n", last_free_bits(0xF8));
	printf("Number: %d\n", last_free_bits(0xF0));
	printf("Number: %d\n", last_free_bits(0xE0));
	printf("Number: %d\n", last_free_bits(0xC0));
	printf("Number: %d\n", last_free_bits(0x80));
	printf("Number: %d\n", last_free_bits(0x00));
}
