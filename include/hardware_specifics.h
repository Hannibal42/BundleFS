/**
 * @file
 * @author  Simon Hanisch <simon@ifsr.de>
 * @version 0.7
 *
 * @section DESCRIPTION
 *
 * This file contains the functions that are specific to a hardware platform,
 * if you want to port the file system to different hardware you have to
 * replace these functions.
 */

#ifndef HARDWARE_SPECIFICS_H
#define HARDWARE_SPECIFICS_H

#include <stdint.h>
#include <sys/time.h>
#include <stddef.h>

/**
 * Returns the system time
 */
uint64_t get_time(void);

/**
 * The size of the check sum, that is used to secure the sectors, in byte.
 */
uint32_t check_size(void);

/**
 *	Calculates the crc sum for a sector. Before the calculation reset_crc must
 *	be called to reset the internal state of the calculation. The sector is
 *	feed to the calculation block by block.
 *	@param[in] value The 4 bytes to be added to the calculation
 *	@return The result of the crc calculation
 */
uint32_t calc_crc(const uint32_t value);

/**
 * Resets the internal state of the crc calculation
 */
void reset_crc(void);

/**
 * Calculates the crc sum for a sequence of lenght bytes. If length % 4 != null
 * a padding is used to complete the calculation.
 * @param[in] input The pointer to the sequence of bytes
 * @param[in] length The length of the byte sequence
 * @return The crc sum
 */
uint32_t calc_crc32_8(uint8_t *input, unsigned int length);

#endif /* HARDWARE_SPECIFICS_H */
