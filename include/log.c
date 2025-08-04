/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "log.h"

/**
 * @brief Format the current time into a log message time strin
 * @param time_str output time string
 * @details Logging helper function: use the format specified in `TIME_FMT` from
 `log.h` on the current time
 */
void format_time(char *time_str)
{
	time_t current_time;
	struct tm tm;

	current_time = time(NULL);
	localtime_r(&current_time, &tm);
	strftime(time_str, MAX_TIME_STR, TIME_FMT, &tm);
}

/**
 * @brief Format a string into a log message
 * @param msg output log message
 * @param fmt message format (excluding timestamp)
 * @param args format string arguments
 * @details Apply the supplied format, prefixing the message with a timestamp
 */
void format_msg(char *msg, char *fmt, va_list args)
{
	char time_str[MAX_TIME_STR], msg_fmt[MAX_LOG_MSG];

	format_time(time_str);
	strncpy(msg_fmt, time_str, MAX_LOG_MSG);

	strncat(msg_fmt, fmt, MAX_LOG_MSG - (strlen(msg_fmt)+1));
	vsnprintf(msg, MAX_LOG_MSG, msg_fmt, args);
}

/**
 * @brief Print error message to stderr
 * @param fmt format string
 * @param ... format arguments
 */
void pr_err(char *fmt, ...)
{
	va_list args;
	char msg[MAX_LOG_MSG];

	va_start(args, fmt);
	format_msg(msg, fmt, args);
	fprintf(stderr, "%s", msg);
	va_end(args);
}

/**
 * @brief Print debug message
 * @details Only runs when compiled with `-DDEBUG`
 * @param fmt format string
 * @param ... format arguments
 */
#ifdef DEBUG
	void pr_debug(char *fmt, ...)
	{
		char msg[MAX_LOG_MSG];
		va_list args;

		va_start(args, fmt);
		format_msg(msg, fmt, args);
		printf("%s", msg);
		va_end(args);
	}
#else
	void pr_debug(char *fmt, ...) { }
#endif
