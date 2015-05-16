#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <time.h>
#include <string.h>
#include "disk_interface.h"


/* Standart start point for the fs */

#define FS_START 0;

/* File system information */ 
struct FILE_SYSTEM{
	char id;
	unsigned long alloc_table; // start of the allocation table
	unsigned long alloc_table_size; //Number of sectors for the allocation table
	unsigned long inode_alloc_table; // start of the inode allocation table
	unsigned long inode_alloc_table_size;
	unsigned long inode_block; // Start of the inode block
	unsigned long inode_block_size;
	unsigned int sector_size;
	struct disk* disk; //Pointer to the disk the fs is belonging to
};

/* File information */
struct INODE{
	unsigned long id; //unique id in the file system
	unsigned int size;
	unsigned int creation_date;  
	unsigned int last_modified;
	unsigned int offset; //Offset into the file beginning from the start
	unsigned int location;  // Pointer to the start sector of the file
	short custody;
	unsigned int time_to_live;
};


/* Modes for opening a file */ 
enum fmode{
	FREAD,
	FWRITE,
	FCREATE
};

/* Modes for fs_seek */
enum SEEK_MODE{
	CUR,
	SET,
	END
};

/* Results of the file functions */
enum FSRESULT{
	FS_OK,
	FS_ERROR,
	FS_PARAM_ERROR
};


enum FSRESULT fs_open(unsigned long number,struct INODE* file,enum fmode mode);
enum FSRESULT fs_create(struct FILE_SYSTEM* fs,struct INODE* inode,
 unsigned long size, unsigned int time_to_live, short custody);
enum FSRESULT fs_close(struct INODE* file);
enum FSRESULT fs_delete(struct INODE* file);
enum FSRESULT fs_read(struct INODE* file,char* buffer,unsigned long size);
enum FSRESULT fs_write(struct INODE* file,char* buffer,unsigned long size);
//enum FSRESULT fs_flush(); //Writes all cached data to the disk


unsigned long fs_tell(struct INODE* file);
enum FSRESULT fs_seek(struct INODE* file,int offset, enum SEEK_MODE mod);

enum FSRESULT fs_mkfs(struct disk* disk/*, uint au */);
enum FSRESULT fs_mount(struct disk* disk,struct FILE_SYSTEM* fs); // Mounts the filesystem
unsigned long fs_getfree(struct disk* disk, struct FILE_SYSTEM* fs);

#endif /* FILE_SYSTEM_H */