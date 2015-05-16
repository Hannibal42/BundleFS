#include "../h/file_system.h"
#include <stdint.h>
#include <time.h>


unsigned long round_up_div(unsigned long dividend, unsigned long divisor);// TODO: Maybe inline
int popcount(unsigned char x); 

enum FSRESULT fs_mkfs(struct disk* disk)
	//uint au //size of allocation unit TODO: Do i want to implement this?)
{

	//Check if the drive is ready
	unsigned int i;
	struct FILE_SYSTEM* fs; // the file system to be created
	unsigned long sector_count, sector_size, max_file_count;
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
    fs->sector_size = sector_size;
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
	//int k;
	//for(k = 0; k < )

	disk_read(disk, buffer, 0, superblock_size);

	/*
	int i;
	for(i = 0; i < (superblock_size * sector_size); ++i){ //TODO:Remove
		printf("%hd ", (short) buffer[i]);
	}
	printf("\n");
	*/

	//memcpy(buffer, fs , sizeof(struct FILE_SYSTEM));
	struct FILE_SYSTEM* tempFS = (struct FILE_SYSTEM*) buffer; //This is ugly but memcpy makes fuuuu
	fs->alloc_table = tempFS->alloc_table;
	fs->alloc_table_size= tempFS->alloc_table_size;
	fs->inode_alloc_table= tempFS->inode_alloc_table;
	fs->inode_alloc_table_size= tempFS->inode_alloc_table_size;
	fs->inode_block= tempFS->inode_block;
	fs->inode_block_size= tempFS->inode_block_size;
	fs->sector_size = tempFS->sector_size;
	
	/*
	char* buffer2;
	buffer2 = (char*) fs;
	for(i = 0; i < sizeof(struct FILE_SYSTEM); ++i){ //TODO:Remove
		printf("%hd ", (short) buffer[i]);
	}
	printf("\n");

	*/

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
	char* buffer;
	unsigned int i;

	if(disk_ioctl(disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		return FS_ERROR;
	}

	//Get number of sectors 
	if(disk_ioctl(disk, GET_SECTOR_COUNT, &sector_count) != RES_OK){
		return FS_ERROR;
	}
	byte_count = round_up_div(sector_count, 8);

	buffer = (char*) malloc(fs->alloc_table_size * sector_size);
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

int get_first_free_bit(uint8_t byte){
	int i;
	uint8_t temp= 0xFF;
	for(i = 8; i >= 0; --i){
		if((byte & (temp << i)) != 0x00)
			return (i * (-1)) + 7;
	}
	return 8;
}

int get_last_free_bit(uint8_t byte){
	int i;
	uint8_t temp= 0xFF;
	for(i = 8; i >= 0; --i){
		if((byte & (temp >> i)) != 0x00)
			return (i * (-1)) + 7;
	}
	return 8;
}
/*
enum FSRESULT write_data(struct FILE_SYSTEM* fs, char* buffer, unsigned int size, unsigned int offset)
{
	unsigned int sector_count = round_up_div(offset + size, fs->sector_size);
	unsigned int i; 
	char* read_buffer; //Read buffer for the first an last block;
	read_buffer = (char*) malloc(fs->sector_size);	

	disk_read(fs->disk, read_buffer, 
	//Anfang
	if(sector_count > 1){

	}else{
		for(i = 0; i < size; ++i){

		}
	}


} */

enum FSRESULT fs_create(struct FILE_SYSTEM* fs,struct INODE* inode,
 unsigned long size, unsigned int time_to_live, short custody)
{
	unsigned long sector_size;
	unsigned int allocation_offset;

	if(disk_ioctl(fs->disk, GET_SECTOR_SIZE, &sector_size) != RES_OK){
		return FS_ERROR;
	}
	char* alloc_table = malloc(fs->alloc_table_size * sector_size);
	disk_read(fs->disk, alloc_table, fs->alloc_table, fs->alloc_table_size);

	int i,k, bit_count, byte_count;

	bit_count = round_up_div(size, sector_size);
	byte_count = round_up_div(bit_count,8);

	uint8_t* inode_alloc_table = malloc(fs->inode_alloc_table_size * sector_size);
	disk_read(fs->disk, (char*)inode_alloc_table, fs->inode_alloc_table, fs->inode_alloc_table_size);

	int bytes_inode_alloc_table = (fs->inode_alloc_table_size * sector_size); //TODO: Naming
	uint8_t temp3 = 0; 

	unsigned int inode_offset = 0;
	for(i = 0; i < bytes_inode_alloc_table; ++i){
		if(inode_alloc_table[i] != 0xFF)
			break;
	}

	//No free inodes
	if((i >= bytes_inode_alloc_table) && (inode_alloc_table[i-1] == 0xFF)){
		printf("Error \n");
		return FS_ERROR;
	}

	for(k = 0; k < 8; ++k){
		temp3 = 0x80 >> k;
		if((inode_alloc_table[i] & temp3) == 0x00){
			inode_alloc_table[i] = inode_alloc_table[i] | temp3;
			inode_offset = (8 * i) + k;
			break;
		}
	}
	disk_write(fs->disk, (char*) inode_alloc_table, fs->inode_alloc_table, fs->inode_alloc_table_size);
	free(inode_alloc_table);
	
	printf("inode offset: %d \n",inode_offset); //TODO:Remove

	//Finds a sequence of bits that are empty in the allocation table
	k = 0;
	int temp_bit_count = 0;
	while(k < (fs->alloc_table_size * sector_size)){
		if((alloc_table[k] & 0xFF) == 0x00){
			temp_bit_count += 8;
		}
		else{
			//Anfang im byte
			if(temp_bit_count == 0){
				temp_bit_count += get_last_free_bit(alloc_table[k]);
			}
			else{
				//Ende im byte
				temp_bit_count += get_first_free_bit(alloc_table[k]);
				if(temp_bit_count >= bit_count)
					break;
				temp_bit_count = 0;
			}
		}
		//Ende normal
		if(temp_bit_count >= bit_count)
			break;
		k += 1;
	}

	if(temp_bit_count < bit_count){
		free(alloc_table);
		//TODO: Das bit in der inode table wieder freigeben
		return FS_ERROR;
	}

	k = (k - byte_count) + 1;


	temp_bit_count = bit_count;
	int startpadding = get_last_free_bit(alloc_table[k]);
	allocation_offset = (8 * k) + (8 - startpadding);
	printf("Allocation offset: %d \n", allocation_offset); //TODO: remove
	uint8_t temp2;
	//Writes the bits into the alloc_table
	for(i = k; i < (k + byte_count); ++i){
		//Anfang
		if(temp_bit_count == bit_count){
			if(temp_bit_count > startpadding){
				temp2 = 0xFF >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] | temp2;
				temp_bit_count -= startpadding;
			} else{
				temp2 = 0xFF << (8 - temp_bit_count);
				temp2 = temp2 >> (8 - startpadding);
				alloc_table[i] = alloc_table[i] | temp2;
			}
		} 
		else {
			//Ende 
			if(i == (byte_count - 1)){
				temp2 = 0xFF << (8 - temp_bit_count);
				alloc_table[i] = alloc_table[i] | temp2;
			} 
			//Mitte
			else{
				alloc_table[i] = 0xFF;
				temp_bit_count -= 8;
			}
		}
	} 

	disk_write(fs->disk, alloc_table, fs->alloc_table, fs->alloc_table_size);
	free(alloc_table);

	inode->size = size;
	inode->creation_date = (unsigned int) time(NULL); //TODO: Get Time;
	inode->last_modified = (unsigned int) time(NULL);
	inode->offset = 0;
	inode->location = allocation_offset;
	inode->custody = 0;
	inode->time_to_live = time_to_live;

	int sector_number = 0;
	int offset_into_sector = 0;
	char* inode_buf = (char *) inode;
	sector_number = (inode_offset * sizeof(struct INODE)) / sector_size;
	offset_into_sector = (inode_offset * sizeof(struct INODE)) % sector_size;

	//Inode is split into two sectors. TODO: Make general function for writing into two sectors
	if((offset_into_sector + sizeof(struct INODE)) > sector_size){
		printf("Needs to be split");
		unsigned int sectors_needed =  

		return FS_ERROR;
	} else{
		char* buffer3 = malloc(sector_size);
		disk_read(fs->disk, buffer3, sector_number, sector_size);

		for(i = 0; i < sizeof(struct INODE); ++i){
			buffer3[offset_into_sector + i] = inode_buf[i];
		}

		disk_write(fs->disk, buffer3, sector_number, sector_size);
		free(buffer3);
	}

	return FS_OK; 
}

enum FSRESULT fs_seek(struct INODE* file,int offset, enum SEEK_MODE mod)
{
	unsigned int temp;
	switch(mod){
		case SEEK_CUR:
			temp = file->offset + offset;
			if(temp < file->size){//TODO: Kleiner oder groesser gleich ?
				file->offset = temp;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		case SEEK_SET:
			if(offset < file->size && offset >= 0){
				file->offset=offset;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		case SEEK_END:
			temp = file->offset + offset;
			if(temp < file->size && file->offset <= offset){//TODO: Kleiner oder groesser gleich ?
				file->offset = temp;
				return FS_OK;
			}
			return FS_PARAM_ERROR;
		default:
			return FS_PARAM_ERROR;
	}
}

unsigned long fs_tell(struct INODE* file)//TODO: Wie verhindere ich das irgendwer einfach in der Inode rumschreibt?
{
	return (long) file->offset;
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
	
	print_fs(fs);
	printf("%lu \n", fs_getfree(disk2,fs));
	disk_shutdown(disk2);

	free(disk2);
	free(disk);
	free(fs);
	return 0; 
}