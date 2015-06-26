#ifndef BUFFER_H
#define BUFFER_H

#include "fs_defines.h"

/* Note: The unused warning is suppressed for these buffers, works only for gcc */
static uint8_t __attribute__((__unused__)) AT_BUFFER[AT_SIZE];
static uint8_t __attribute__((__unused__)) IT_BUFFER[AT_SIZE];
static uint8_t __attribute__((__unused__)) SEC_BUFFER[SECTOR_SIZE];

#endif /* BUFFER_H */