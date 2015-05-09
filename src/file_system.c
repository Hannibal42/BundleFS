#include "../h/file_system.h"


unsigned long round_div(unsigned long dividend, unsigned long divisor);// TODO: Maybe inline

enum FSRESULT fs_mkfs(struct disk* disk)
	//uint au //size of allocation unit TODO: Do i want to implement this?)
{

	//Check if the drive is ready
	unsigned int i;
	enum DRESULT dsksta; // used to store temporary output
	struct FILE_SYSTEM* fs; // the file system to be created
	unsigned long sector_count, sector_size, disk_size, max_file_count;
	unsigned long alloc_table_size, inode_alloc_table_size, inode_block_size;
	fs = (struct FILE_SYSTEM*) malloc(sizeof(struct FILE_SYSTEM));
	//dsksta = disk_initialize(disk);
	//if(dsksta != RES_OK){
	//	printf("Fehler3\n");TODO: I dont think i have to initialize again?
	//	return FS_ERROR;
	//}
	//Gets the sector size
	if(disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		printf("Fehler2\n");
		return FS_ERROR;
	}

	//Get number of sectors 
	if(disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK){
		printf("Fehler1\n");
		return FS_ERROR;
	}
	//Calculate the disk size
	disk_size = sector_count * sector_size;
	//Calculate the fs size //Make fs_info
	max_file_count = sector_count; //TODO:What should be me maximum file count?
	alloc_table_size = round_div(sector_count, sector_size);           //TODO: Fill up the end of the alloc table with 1
    fs->alloc_table_size = alloc_table_size;
    fs->alloc_table = 1; // The first sector is reserved for the superblock; 
    unsigned long size = sector_size * alloc_table_size;
    unsigned long dif = size -(sector_count % sector_size);
    char* temparray = malloc(sizeof(char) * size);
    for(i = 0; i < size; ++i){
    	if(i < dif){
    		temparray[i] = 0x0;
    	} else {
    		temparray[i] = 0x01;
    	}
    }

    for(i =0; i < size; ++i){//TODO: Remove
    	printf("%s",temparray);
    }
    printf("\n");
    if(disk == NULL)
    	printf("Fehler");
    printf("%lu \n",fs->alloc_table);
    printf("%lu \n", alloc_table_size);
    disk_write(disk,temparray,fs->alloc_table, alloc_table_size);
    
    /*for(i =0; i < size; ++i){//TODO: Remove
    	printf("%hu",temparray[i]);
    }
    printf("\n"); */ 

    inode_alloc_table_size = round_div(max_file_count, sector_size); //TODO: Fill up the end of the alloc table with 1
    fs->inode_alloc_table_size = inode_alloc_table_size;

    inode_block_size = round_div(max_file_count * sizeof(struct INODE), sector_size);
    fs->inode_block_size = inode_block_size;
    
    // Clear the fsblock area
    
	//Make allocation table

	//Make inode allocation table
	
	//Make inode block
	//Make superblock


}



unsigned long round_div(unsigned long dividend, unsigned long divisor)
{
    return (dividend + divisor - 1) / divisor;
}

/*
int main()
{ /*
	struct disk* disk;
	disk = make_disk("test.disk");
	disk_initialize(disk);
	char buff[64];
	int i;
	for(i = 3; i < 64; ++i){
		buff[i] = 0xFF;
	}
	disk_write(disk, buff, 1, 1);


	//fs_mkfs(disk);

	disk_shutdown(disk);
	free(disk);
	return 0; 
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
}*/