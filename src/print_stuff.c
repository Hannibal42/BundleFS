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


void print_inode(struct INODE* file)
{
	printf("ID: %lu \n", file->id);
	printf("Size: %d \n", file->size);
	printf("creation date: %d \n", file->creation_date);
	printf("last modified: %d \n", file->last_modified);
	printf("offset: %d \n", file->offset);
	printf("location: %d \n", file->location);
	printf("inode offset: %d \n", file->inode_offset);
	printf("custody: %hu \n", file->custody);
	printf("TTL: %d \n", file->time_to_live);
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

	struct INODE* inode1;
	struct INODE* inode2;
	struct INODE* inode3;
	struct INODE* inode4;
	inode1 = (struct INODE*) malloc(sizeof(struct INODE));
	inode2 = (struct INODE*) malloc(sizeof(struct INODE));
	inode3 = (struct INODE*) malloc(sizeof(struct INODE));
	inode4 = (struct INODE*) malloc(sizeof(struct INODE));
	inode1->id=0; 
	inode2->id=1;
	inode3->id=2;
	inode4->id=3;

	fs_create(fs,inode1,30,1000,1);
	fs_create(fs,inode2,30,1000,1);
	fs_create(fs,inode3,30,1000,1);
	fs_create(fs,inode4,30,1000,1);

	free(inode1);

	inode1 = NULL;

	inode1 = (struct INODE*) malloc(sizeof(struct INODE));
	fs_open(fs, 0, inode1);

	char* buffer;
	buffer = malloc(fs->sector_size);
	buffer[0] = 0xFF;
	buffer[1] = 0x00;
	buffer[2] = 0xFF;
	buffer[3] = 0x00;

	fs_write(fs, inode1, buffer);
	print_inode(inode1);
	print_inode(inode2);
	fs_delete(fs, inode1);

	free(inode1);
	free(inode2);
	free(inode3);
	free(inode4);

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
	printf("Free: %lu \n", fs_getfree(disk2,fs));

	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0; 
}