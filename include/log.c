#include <stdio.h>
#include <stdarg.h>

/* TODO message prefixes */

void pr_err(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

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

void pr_info(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
