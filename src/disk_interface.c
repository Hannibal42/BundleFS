#include "disk_interface.h"


enum DISK_STATUS disk_status(const struct disk *disk)
{
	return disk->status;
}

enum DRESULT disk_initialize(struct disk *disk)
{
	if (!disk)
		return RES_PARERR;

	disk->file = fopen(disk->file_name, "r+b");

	if (disk->file) {
		disk->status = STA_READY;
		return RES_OK;
	}
	disk->status = STA_NOINIT;
	return RES_ERROR;
}

enum DRESULT disk_shutdown(struct disk *disk)
{
	if (!disk || disk->status != STA_READY)
		return RES_PARERR;

	fclose(disk->file);
	disk->status = STA_NOINIT;
	return RES_OK;
}

enum DRESULT disk_read(struct disk *disk, char *buff, uint sector,
	uint number_of_sectors)
{
	if (!disk)
		return RES_PARERR;

	unsigned long number_of_bytes = disk->sector_size * number_of_sectors;

	if (number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, sector * disk->sector_size, SEEK_SET);
	fread(buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

enum DRESULT disk_write(struct disk *disk, char *buff, uint sector,
	uint number_of_sectors)
{
	if (!disk)
		return RES_PARERR;

	int number_of_bytes = (int) disk->sector_size * (int) number_of_sectors;

	if (number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, sector * disk->sector_size, SEEK_SET);
	fwrite(buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

time_t system_getTime(void)
{
	return time(NULL);
}

enum DRESULT disk_ioctl(struct disk *disk, char cmd, unsigned long *buff)
{
	switch (cmd) {
	case CTRL_SYNC:
		/*Flush the disk here*/
		return RES_ERROR;
	case GET_SECTOR_COUNT:
		(*buff) = disk->sector_count;
		return RES_OK;
	case GET_SECTOR_SIZE:
		(*buff) = disk->sector_size;
		return RES_OK;
	case CTRL_ERASE_SECTOR:
		/* Erase a sector */
		return RES_ERROR;
	default:
		return RES_ERROR;
	}
}
