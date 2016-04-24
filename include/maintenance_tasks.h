/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the maintenance tasks for the file system.
 */

#ifndef MAINTENANCE_TASKS_H
#define MAINTENANCE_TASKS_H

#include "file_system.h"
#include "inode_functions.h"
#include "window_buffer.h"

/**
 * This functions scans the file system for inodes that are no longer valid
 * because their time to live expired. The invalid inodes are deleted.
 * @param[in] fs The struct holding the file system metadata
 */
void delete_invalid_inodes(struct FILE_SYSTEM *fs);

/**
 * Defragments the file systems, should be used from time to time.
 * @param[in] fs The struct holding the file system metadata
 */
void defragment(struct FILE_SYSTEM *fs);

/**
 * If the file system is not shut down properly it can happen that sectors
 * are marked used that are actually free. This functions frees those sectors.
 * @param[in] fs The struct holding the file system metadata
 */
void restore_fs(struct FILE_SYSTEM *fs);

#endif /* MAINTENANCE_TASKS_H */
