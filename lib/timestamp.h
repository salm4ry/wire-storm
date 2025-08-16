/**
 * @file timestamp.h
 * @brief Functions relating to timestamps
 */

#include <stdbool.h>
#include <time.h>

void get_clock_time(struct timespec *timestamp);
bool compare_times(struct timespec *lhs, struct timespec *rhs);
