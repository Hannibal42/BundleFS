#ifndef TASKS_H
#define TASKS_H

#include "file_system.h"

void delete_invalid_inodes(struct FILE_SYSTEM *fs);
void defragment(struct FILE_SYSTEM *fs);

#endif /* TASKS_H */
