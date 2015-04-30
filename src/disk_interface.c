#include "../h/disk_interface.h"


enum DISK_STATUS disk_status(const struct disk* disk)
{
	return disk->status; 
}

enum DRESULT disk_initialize(struct disk* disk)
{
	
	if(!disk)
		return RES_PARERR;

	disk->file = fopen(disk->file_name, "w+b");
	short buffer[1024] = {0};
	fseek(disk->file, 0, SEEK_SET);
	fwrite((char*)buffer, sizeof(char), 1024, disk->file);//TODO: Change this after testing
	fclose(disk->file);

	disk->file = fopen(disk->file_name, "r+b");

	if(!disk->file){
		disk->status = STA_READY;
		return RES_OK;
	}
	return RES_ERROR;
}

enum DRESULT disk_shutdown(struct disk* disk)
{
	if(!disk || disk->status != STA_READY)
		return RES_PARERR;

	fclose(disk->file);
	disk->status = STA_NOINIT;
	return RES_OK;
}

enum DRESULT disk_read(struct disk* disk, char* buff, int block, int number_of_blocks)
{
	if(!disk)
		return RES_PARERR;
	
	int number_of_bytes = disk->block_size * number_of_blocks;

	if(number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, block * disk->block_size ,SEEK_SET);
	fread(buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

//You need to check if the buffer has the right size, otherwise your writing out of bound.
enum DRESULT disk_write(struct disk* disk, char* buff, int block, int number_of_blocks)
{
	if(!disk)
		return RES_PARERR;

	int number_of_bytes = disk->block_size * number_of_blocks;

	if(number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, block * disk->block_size ,SEEK_SET);
	fwrite(buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

time_t system_getTime()
{
	return time(NULL);
}


int main()
{
	struct disk* disk;
	disk = make_disk("test.disk");
	disk_initialize(disk);
	char buff[64];
	buff[0] = 'A';
	buff[1] = 'B';
	buff[2] = 'C';
	int i;
	for(i = 3; i < 64; ++i){
		buff[i] = '0';
	}
	disk_write(disk, buff, 1, 1);
	char buff2[64] = {0};
	disk_read(disk,buff2,1,1);
	disk_write(disk,buff2,2,1);
	disk_shutdown(disk);
	free(disk);

	return 0;
}
