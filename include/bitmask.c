/// @file

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
 * @brief Update a bit at a given position
 * @param mask bitmask to update
 * @param pos position of bit to update
 * @param val value to set the bit to
 */
void update_bit(uint64_t *mask, int pos, bool val)
{
	if (val) {
		*mask |= (1 << pos);
	} else {
		*mask &= ~(1 << pos);
	}
}
