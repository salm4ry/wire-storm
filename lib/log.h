/// @file

#include <stdarg.h>

#define MAX_TIME_STR 21  ///< maximum length for log message timestamp
#define TIME_FMT "%Y-%m-%d %H:%M:%S " ///< log message timestamp format
#define MAX_LOG_MSG 128  ///< maximum log message length

void format_time(char *time_str);
void format_msg(char *msg, char *fmt, va_list args);

void pr_err(char *fmt, ...);
void pr_debug(char *fmt, ...);
