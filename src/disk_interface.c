#include "disk_interface.h"


enum DISK_STATUS disk_status(const struct DISK *disk)
{
	return disk->status;
}

enum DRESULT disk_initialize(struct DISK *disk)
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

enum DRESULT disk_shutdown(struct DISK *disk)
{
	if (!disk || disk->status != STA_READY)
		return RES_PARERR;

	fclose(disk->file);
	disk->status = STA_NOINIT;
	return RES_OK;
}

enum DRESULT disk_read(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors)
{
	if (!disk)
		return RES_PARERR;

	sector *= disk->sector_block_mapping;
	number_of_sectors *= disk->sector_block_mapping;

	unsigned long number_of_bytes = disk->block_size * number_of_sectors;

	if (number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, sector * disk->block_size, SEEK_SET);
	fread((char *) buff, sizeof(char), number_of_bytes, disk->file);

	return RES_OK;
}

enum DRESULT disk_write(struct DISK *disk, uint8_t *buff, uint sector,
	uint number_of_sectors)
{
	if (!disk)
		return RES_PARERR;

	sector *= disk->sector_block_mapping;
	number_of_sectors *= disk->sector_block_mapping;

	int number_of_bytes = (int) disk->block_size * (int) number_of_sectors;

	if (number_of_bytes > disk->size)
		return RES_PARERR;

	fseek(disk->file, sector * disk->block_size, SEEK_SET);
	fwrite((char *) buff, sizeof(char), number_of_bytes, disk->file);
	/* Writes the cache to disk */
	fflush(disk->file);

	return RES_OK;
}

time_t system_getTime(void)
{
	return time(NULL);
}

enum DRESULT disk_ioctl(struct DISK *disk, char cmd, unsigned long *buff)
{
	switch (cmd) {
	case CTRL_SYNC:
		/*Flush the disk here*/
		return RES_ERROR;
	case GET_SECTOR_COUNT:
		(*buff) = disk->block_count;
		return RES_OK;
	case GET_SECTOR_SIZE:
		(*buff) = disk->block_size;
		return RES_OK;
	case CTRL_ERASE_SECTOR:
		/* Erase a sector */
		return RES_ERROR;
	default:
		return RES_ERROR;
	}
}
