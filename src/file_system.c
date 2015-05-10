#include "../h/file_system.h"


unsigned long round_up_div(unsigned long dividend, unsigned long divisor);// TODO: Maybe inline

enum FSRESULT fs_mkfs(struct disk* disk)
	//uint au //size of allocation unit TODO: Do i want to implement this?)
{

	//Check if the drive is ready
	unsigned int i;
	enum DRESULT dsksta; // used to store temporary output
	struct FILE_SYSTEM* fs; // the file system to be created
	unsigned long sector_count, sector_size, disk_size, max_file_count;
	unsigned long size, dif;
	unsigned long superblock_size; //Size of the superblock in sectors
	char* temparray; //Used to make the fs superblock
	fs = (struct FILE_SYSTEM*) malloc(sizeof(struct FILE_SYSTEM));

	//Gets the sector size
	if(disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		return FS_ERROR;
	}

	superblock_size = round_up_div(sizeof(struct FILE_SYSTEM),sector_size);

	//Get number of sectors 
	if(disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK){
		return FS_ERROR;
	}
	//Calculate the fs size //Make fs_info
	//Make the allocation table.
	max_file_count = sector_count / 8; //TODO:What should be me maximum file count? TODO:Change this to a better thing
	
	fs->alloc_table_size = round_up_div(sector_count, sector_size * 8); //The number of sectors needed for the allocation table.
    fs->alloc_table = superblock_size; //first comes the superblock; 
    size = sector_size * fs->alloc_table_size;
    //printf("Sector_count: %lu \n",sector_count);
    //printf("alloc_table_size: %lu \n", fs->alloc_table_size);
    dif = round_up_div(sector_count,8) % sector_size; //Calculates the padding of the sector
    dif = (fs->alloc_table_size - 1) * sector_size + dif;
    //printf("Dif: %lu \n",dif);
    if(sector_count % 8 != 0) //TODO:This should never happen if you used the right numbers for the fs
    	dif = dif - 1;
    temparray = malloc(sizeof(char) * size);
    for(i = 0; i < size; ++i){
    	if(i >= dif){
    		temparray[i] = 0xFF;
    	} else {
    		temparray[i] = 0x00;
    	}
    }
    
    disk_write(disk,temparray,fs->alloc_table, fs->alloc_table_size);
    free(temparray);

    //Make inode_alloc_table;
    fs->inode_alloc_table_size = round_up_div(max_file_count, sector_size * 8);
    fs->inode_alloc_table = fs->alloc_table + fs->alloc_table_size;

    size = sector_size * fs->inode_alloc_table_size;
    //printf("Sector_count: %lu \n",sector_count);
    //printf("inode_alloc_table_size: %lu \n", fs->inode_alloc_table_size);
    dif = round_up_div(max_file_count, 8)% sector_size;
    dif = (fs->inode_alloc_table_size - 1) * sector_size + dif;
    //printf("Dif: %lu \n",dif);
    if(sector_count % 8 != 0) //TODO: This should never happen.
    	dif = dif - 1;
    temparray = malloc(sizeof(char) * size);
    for(i = 0; i < size; ++i){
    	if(i >= dif){
    		temparray[i] = 0xFF;
    	} else {
    		temparray[i] = 0x00;
    	}
    }

    disk_write(disk, temparray, fs->inode_alloc_table, fs->inode_alloc_table_size);
    free(temparray);

    //Make Inode Block
    fs->inode_block_size = round_up_div(max_file_count * sizeof(struct INODE), sector_size);
    fs->inode_block = fs->inode_alloc_table + fs->inode_alloc_table_size;
    size = sector_size * fs->inode_block_size;
    temparray = malloc(sizeof(struct INODE) * size);
    for(i = 0; i < size; ++i){
    		temparray[i] = 0x0;
    }

    disk_write(disk, temparray, fs->inode_block, fs->inode_block_size);
    free(temparray);
    //printf("%lu", sizeof(struct FILE_SYSTEM));
	//Make superblock
	disk_write(disk, (char*) fs, 0, superblock_size); //TODO: Make a padding, so no random memory is writen to disk.
	free(fs);
	return FS_OK;

}


enum FSRESULT fs_mount(struct disk* disk, struct FILE_SYSTEM* fs)
{

	unsigned long sector_size, superblock_size;
	char* buffer;

	if(disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		return FS_ERROR;
	}

	superblock_size = round_up_div(sizeof(struct FILE_SYSTEM),sector_size);
	buffer = (char*) malloc(superblock_size * sector_size);

	disk_read(disk, buffer, 0, superblock_size);

	memcpy(buffer, fs , sizeof(struct FILE_SYSTEM));
	free(buffer);

	return FS_OK;
}

//stolen from the internet, counts the bits in a byte
int popcount(unsigned char x)
{
	return ((0x876543210 >>
    (((0x4332322132212110 >> ((x & 0xF) << 2)) & 0xF) << 2)) >>
    ((0x4332322132212110 >> (((x & 0xF0) >> 2)) & 0xF) << 2))
    & 0xf;
}


unsigned long fs_getfree(struct disk* disk, struct FILE_SYSTEM* fs)
{
	unsigned long sector_size, ret, sector_count, byte_count;
	unsigned char* buffer;
	unsigned int i;

	if(disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		return FS_ERROR;
	}

	//Get number of sectors 
	if(disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK){
		return FS_ERROR;
	}
	byte_count = round_up_div(sector_count, 8);

	buffer = (unsigned char*) malloc(fs->alloc_table_size * sector_size);
	disk_read(disk,buffer, fs->alloc_table, fs->alloc_table_size);

	for(i = 0; i < byte_count; ++i){
		printf("%hd", (short) buffer[i]);
	}
	printf("\n");

	ret = 0;
	for(i = 0; i < byte_count; ++i){
		if(buffer[i] != 0){
			ret += popcount(buffer[i]);
		}
	}
	free(buffer);
	return ret;
}




inline unsigned long round_up_div(unsigned long dividend, unsigned long divisor)
{
    return (dividend + divisor - 1) / divisor;
}

void print_fs(struct FILE_SYSTEM* fs)
{
	printf("Alloc_table:%lu \n", fs->alloc_table);
	printf("Alloc_table_size:%lu \n", fs->alloc_table_size);
	printf("Inode_Alloc_table:%lu \n", fs->inode_alloc_table);
	printf("Inode_Alloc_table_size:%lu \n", fs->inode_alloc_table_size);
	printf("inode_block:%lu \n", fs->inode_block);
	printf("inode_block_size:%lu \n", fs->inode_block_size);
}

int main()
{ 
	struct disk* disk;
	disk = make_disk("test.disk");
	disk_initialize(disk);

	fs_mkfs(disk);
	struct FILE_SYSTEM* fs;
	fs = malloc(sizeof(struct FILE_SYSTEM));
	fs->disk = disk; 
	fs_mount(disk,fs);
	print_fs(fs);
	printf("%lu \n", fs_getfree(disk,fs));
	disk_shutdown(disk);

	free(disk);
	free(fs);
	return 0; 
}