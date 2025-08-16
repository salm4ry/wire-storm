/**
 * @file bitmask.c
 * @brief Definitions of functions for checking and setting bits in 64-bit masks
 */

#include "bitmask.h"

/**
 * @brief Determine if a given bit is set
 * @param mask bitmask to check
 * @param pos position of bit to check
 * @return True if the bit is set (1), false otherwise
 */
bool is_set(uint64_t *mask, int pos)
{
	return ((*mask) & (1 << pos));
}

/**
 * @brief Set a given bit to a given value
 * @param mask bitmask to update
 * @param pos position of bit to update
 * @param val value to set the bit to
 */
void set_bit(uint64_t *mask, int pos, bool val)
{
	if (val) {
		*mask |= (1 << pos);
	} else {
		*mask &= ~(1 << pos);
	}
}
