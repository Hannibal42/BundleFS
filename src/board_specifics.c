#include "board_specifics.h"

uint64_t get_time(void)
{
	struct timeval t;

	gettimeofday(&t, NULL);

	return (uint64_t) t.tv_sec;
}
/*
unsigned int check_size(void)
{
	return sizeof(uint32_t);
}*/

/*uint32_t calc_crc32_8(uint8_t *input, unsigned int lenght)
{
	CRC->CR = CRC_CR_RESET;
	uint32_t i, len;
	uint32_t *tmp;

	tmp = (uint32_t *) input;
	len = lenght / 4;
	for (i = 0; i < len; ++i)
		CRC->DR = tmp[i];
	*/
	/* With a normal sector size this will never happen */
	/*for (i = 0; i < (lenght % 4); ++i)
		CRC->DR = input[lenght - (4 - i)];

	return CRC->DR;
}*/


/* TODO: Change this function for upcn*/
uint32_t check_size(void)
{
	return sizeof(uint32_t);
}

uint32_t calc_fake_crc(const uint32_t value)
{
	static uint32_t crc;

	crc ^= value;
	return crc;
}

void reset_fake_crc(void)
{
	uint32_t tmp;

	tmp = calc_fake_crc(0x00);
	calc_fake_crc(tmp);
}

