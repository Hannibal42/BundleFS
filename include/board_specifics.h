#ifndef BOARD_SPECIFICS_H
#define BOARD_SPECIFICS_H

//#include "stm32f4xx_crc.h"
//#include "upcn.h"
#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>

/*
uint32_t calc_crc32_8(uint8_t *input, unsigned int lenght);

unsigned int check_size(void);
*/

uint64_t get_time(void);

/* TODO Change this */
uint32_t check_size(void);
uint32_t calc_fake_crc(const uint32_t value);
void reset_fake_crc(void);

#endif /* BOARD_SPECIFICS_H */
