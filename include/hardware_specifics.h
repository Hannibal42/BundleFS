#ifndef HARDWARE_SPECIFICS_H
#define HARDWARE_SPECIFICS_H

#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>

uint64_t get_time(void);
uint32_t check_size(void);
uint32_t calc_fake_crc(const uint32_t value);
void reset_fake_crc(void);
uint32_t calc_crc32_8(uint8_t *input, unsigned int lenght);

#endif /* HARDWARE_SPECIFICS_H */
