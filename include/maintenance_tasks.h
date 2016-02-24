#ifndef MAINTENANCE_TASKS_H
#define MAINTENANCE_TASKS_H

#include "file_system.h"
#include "inode_functions.h"

void delete_invalid_inodes(struct FILE_SYSTEM *fs);
void defragment(struct FILE_SYSTEM *fs);
void restore_fs(struct FILE_SYSTEM *fs);

#endif /* MAINTENANCE_TASKS_H */
