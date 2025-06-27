/// @file

#include <stdio.h>
#include <stdarg.h>


/**
 * @brief Print error message to stderr
 * @param fmt format string
 * @param ... format arguments
 */
void pr_err(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
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
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
#else
	void pr_debug(char *fmt, ...) { }
#endif
