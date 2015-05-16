#include "../h/print_stuff.h"

void print_fs(struct FILE_SYSTEM* fs)
{
	printf("Alloc_table:%lu \n", fs->alloc_table);
	printf("Alloc_table_size:%lu \n", fs->alloc_table_size);
	printf("Inode_Alloc_table:%lu \n", fs->inode_alloc_table);
	printf("Inode_Alloc_table_size:%lu \n", fs->inode_alloc_table_size);
	printf("inode_block:%lu \n", fs->inode_block);
	printf("inode_block_size:%lu \n", fs->inode_block_size);
}

void print_disk(struct disk* disk)
{
	printf("Number:%c \n", disk->number);
	printf("size:%lu \n", disk->size);
	printf("sector_size:%lu \n", disk->sector_size);
	printf("sector_count:%lu \n", disk->sector_count);
	switch(disk->status){
		case STA_NOINIT:
			printf("Status: STA_NOINIT");
			break;
		case STA_NODISK:
			printf("Status: STA_NODISK");
			break;
		case STA_PROTECT:
			printf("Status: STA_PROTECT");
			break;
		case STA_READY:
			printf("Status: STA_READY");
			break;
		case STA_ERROR_NO_FILE:
			printf("Status: STA_ERROR_NO_FILE");
			break;
		default:
			printf("Status: ERROR");
	}
	printf("\n");
}

int main()
{ 
	struct disk* disk;
	struct disk* disk2;
	disk = make_disk("test.disk");
	//disk_create(disk); //creates the file for the disk;
	disk_initialize(disk);
	print_disk(disk);

	fs_mkfs(disk);
	disk_shutdown(disk);
	disk2 = make_disk("test.disk");
	disk_initialize(disk2);
	struct FILE_SYSTEM* fs;
	fs = malloc(sizeof(struct FILE_SYSTEM));
	fs->disk = disk; 
	fs_mount(disk2,fs);
	fs->disk = disk2; 

	struct INODE* i1;
	i1 = (struct INODE*) malloc(sizeof(struct INODE));

	fs_create(fs,i1,30,1000,1);
	fs_create(fs,i1,30,1000,1);
	fs_create(fs,i1,30,1000,1);
	fs_create(fs,i1,30,1000,1);

	free(i1);

	/*
	printf("Number: %d \n", get_first_free_bit(0xFF));
	printf("Number: %d \n", get_first_free_bit(0x7F));
	printf("Number: %d \n", get_first_free_bit(0x3F));
	printf("Number: %d \n", get_first_free_bit(0x1F));
	printf("Number: %d \n", get_first_free_bit(0x0F));
	printf("Number: %d \n", get_first_free_bit(0x07));
	printf("Number: %d \n", get_first_free_bit(0x03));
	printf("Number: %d \n", get_first_free_bit(0x01));
	printf("Number: %d \n", get_first_free_bit(0x00));

	printf("Number: %d \n", get_last_free_bit(0xFF));
	printf("Number: %d \n", get_last_free_bit(0xFE));
	printf("Number: %d \n", get_last_free_bit(0xFC));
	printf("Number: %d \n", get_last_free_bit(0xF8));
	printf("Number: %d \n", get_last_free_bit(0xF0));
	printf("Number: %d \n", get_last_free_bit(0xE0));
	printf("Number: %d \n", get_last_free_bit(0xC0));
	printf("Number: %d \n", get_last_free_bit(0x80));
	printf("Number: %d \n", get_last_free_bit(0x00));
	*/
	//printf("Inode size:%d \n", sizeof(struct INODE));

	print_fs(fs);
	printf("%lu \n", fs_getfree(disk2,fs));
	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0; 
}