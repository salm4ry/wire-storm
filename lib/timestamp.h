/// @file

#include <stdbool.h>
#include <time.h>

#define NS_PER_SEC 1000000000UL ///< 1 second = 10^9 nanoseconds
#define MS_PER_SEC 1000UL ///< 1 second = 10^3 milliseconds
#define MS_PER_NSEC 1000000UL ///< 1 second = 10^6 nanoseconds

void get_clock_time(struct timespec *timestamp);
bool compare_times(struct timespec *lhs, struct timespec *rhs);
