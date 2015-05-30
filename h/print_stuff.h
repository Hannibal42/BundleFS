#ifndef PRINT_STUFF_H
#define PRTIN_STUFF_H

#include "file_system.h"

void print_fs(struct FILE_SYSTEM *fs);
void print_disk(struct disk *disk);
void print_inode(struct INODE *file);

#endif /* PRINT_STUFF_H */
