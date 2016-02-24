#ifndef FS_DEFINES_H
#define MAINTENANCE_TASKS

/* The size of the allocation tables in byte */
#define AT_SIZE 16000
/* The size of the allocation table buffer in byte (needs to be a multiple of SECTOR_SIZE)*/
#define AT_BUFFER_SIZE 4098
/* The size of one sector in byte */
#define SECTOR_SIZE 4098
/* Defines the minimum number of Inodes for the start */
#define INODE_CNT 8

#endif /* MAINTENANCE_TASKS */
