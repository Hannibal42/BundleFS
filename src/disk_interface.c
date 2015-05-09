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
	fwrite((char*)buffer, sizeof(char), 1024, disk->file);//TODO: Change this after testing, the disk shouldnt be always be wiped
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

enum DRESULT disk_read(struct disk* disk, char* buff, unsigned long sector, unsigned long number_of_sectors)
{
	if(!disk)
		return RES_PARERR;
	
	int number_of_bytes = disk->sector_size * number_of_sectors;

	if(number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, sector * disk->sector_size ,SEEK_SET);
	fread(buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

//You need to check if the buffer has the right size, otherwise your writing out of bound.
enum DRESULT disk_write(struct disk* disk, char* buff, unsigned long sector, unsigned long number_of_sectors)
{
	if(!disk)
		return RES_PARERR;

	int number_of_bytes = (int) disk->sector_size * (int) number_of_sectors;

	if(number_of_bytes > disk->size)
		return RES_PARERR;

	printf("Number of bytes:%i \n", number_of_bytes);
	printf("Sector: %lu \n", sector);
	printf("Sector size: %lu \n", disk->sector_size);
	fseek(disk->file, sector * disk->sector_size ,SEEK_SET);
	int size = fwrite(buff, sizeof(char), number_of_bytes, disk->file);

	printf("%i", size);
	return RES_OK;
}

time_t system_getTime()
{
	return time(NULL);
}

enum DRESULT disk_ioctl(struct disk* disk, char cmd, unsigned long* buff)
{
	switch(cmd){
		case CTRL_SYNC:
			//Flush the disk here
			return RES_ERROR;
		case GET_SECTOR_COUNT:
			(*buff) = disk->sector_count;
			return RES_OK;
		case GET_SECTOR_SIZE:
			(*buff) = disk->sector_size;
			return RES_OK;
		case CTRL_ERASE_SECTOR:
			// Erase a sector
			return RES_ERROR;
		default:
			return RES_ERROR;
	}
}


int main()
{
	struct disk* disk;
	disk = make_disk("test.disk");
	disk_initialize(disk);
	char buff[64];
	buff[0] = 0xFF;
	buff[1] = 0xFF;
	buff[2] = 0xFF;
	int i;
	for(i = 3; i < 64; ++i){
		buff[i] = 0x0;
	}
	disk_write(disk, buff, 1, 1);
	char buff2[64] = {0};
	disk_read(disk,buff2,1,1);
	disk_write(disk,buff2,2,1);
	unsigned long out;
	disk_ioctl(disk, GET_SECTOR_COUNT, &out);
	printf( "%lu\n", out);
	disk_ioctl(disk, GET_SECTOR_SIZE,  &out);
	printf("%lu\n", out);
	disk_shutdown(disk);
	free(disk);
	return 0;
}
