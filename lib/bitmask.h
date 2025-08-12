/// @file

#include <stdbool.h>
#include <stdint.h>

bool is_set(uint64_t *mask, int pos);
void update_bit(uint64_t *mask, int pos, bool val);
