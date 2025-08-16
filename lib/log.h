/// @file

#include <stdio.h>
#include <string.h>  /* required for p_error macro */
#include <stdarg.h>

#define MAX_TIME_STR 21  ///< maximum length for log message timestamp
#define TIME_FMT "%Y-%m-%d %H:%M:%S " ///< log message timestamp format
#define MAX_LOG_MSG 128  ///< maximum log message length

/**
 * @brief Print error message
 * @param msg output message
 * @param err_code error code (converted to message using `strerror()`)
 * @details Includes file, function and line details (hence a macro)
 */
#define p_error(msg, err_code) \
{ \
	fprintf(stderr, "%s:%s:%d: %s: %s\n", \
			__FILE__, __func__, __LINE__, \
			msg, strerror(err_code));\
}

void format_time(char *time_str);
void format_msg(char *msg, char *fmt, va_list args);

void pr_err(char *fmt, ...);
void pr_debug(char *fmt, ...);
