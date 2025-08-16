/**
 * @file bitmask.h
 * @brief Functions for checking and setting bits in 64-bit masks
 */

#include <stdbool.h>
#include <stdint.h>

bool is_set(uint64_t *mask, int pos);
void set_bit(uint64_t *mask, int pos, bool val);
