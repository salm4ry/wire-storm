/**
 * @file timestamp.c
 * @brief Definitions of functions relating to timestamps
 */

#include "timestamp.h"
#include "log.h"

/**
 * @brief Get monotonic clock time
 * @param time output time
 */
void get_clock_time(struct timespec *timestamp)
{
	clock_gettime(CLOCK_MONOTONIC, timestamp);
}

/**
 * @brief Compare two `struct timespec` timestamps
 * @param lhs first timestamp
 * @param rhs second timestamp
 * @return true if `lhs < rhs`, false otherwise
 */
bool compare_times(struct timespec *lhs, struct timespec *rhs)
{
	if (lhs->tv_sec == rhs->tv_sec) {
		/* compare nanoseconds if number of seconds is equal */
		return (lhs->tv_nsec < rhs->tv_nsec);
	} else {
		/* otherwise simply compare seconds */
		return (lhs->tv_sec < rhs->tv_sec);
	}
}
