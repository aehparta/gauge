/*
 * #SOFTWARE_NAME#
 * (copied from debuglib)
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi, duge at IRCnet>
 */

#ifndef _WIN32
#include <execinfo.h>
#endif
#include "debug.h"


#ifdef _WIN32
#define vsnprintf(buf, count, format, argptr) _vsnprintf(buf, count, format, argptr)
#endif


/** file stream, where to print the log */
static FILE *dlog_file = NULL;

/** callback to be used for logging */
static void (*dlog_callback) (const char *file, const char *function, int line, const char *type, const char *message) = NULL;

/** add this string at start of log-line in error-level */
static char el_string[] = "ERROR";

/** add this string at start of log-line in warning-level */
static char wl_string[] = "WARNING";

/** add this string at start of log-line in info-level */
static char il_string[] = "INFO";

/** add this string at start of log-line in debug-level */
static char dl_string[] = "DEBUG";

/** log level */
static int log_level = DD_LOG_DEBUG;

/** whether to print to stderr or not */
static int print_stderr = 1;

/** whether to print to log or not */
static int print_enable = 1;

/**
 *  Initialize log-system.
 */
void dd_log_init(char *file)
{
	/* Set log stream. */
	if (file)
	{
		dlog_file = fopen(file, "w");
	}
	print_stderr = 1;
}

/**
 * Quit log-system.
 */
void dd_log_quit(void)
{
	/* Set log stream. */
	if (dlog_file)
	{
		fclose(dlog_file);
	}
}

/**
 * Initialize log-system to use user defined callback.
 */
void dd_log_init_callback(void (*callback) (const char *file, const char *function, int line, const char *type, const char *message))
{
	/* Set log stream. */
	dlog_file = NULL;
	print_stderr = 0;
	dlog_callback = callback;
}

/**
 * Set logging level.
 */
void dd_log_set_level(int level)
{
	log_level = level;
}

/**
 * Print string to LOG. Use like printf().
 */
void dd_log(const char *string, ...)
{
	va_list args;
	char buf[512];

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s", buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s", buf);
		}
		if (dlog_callback)
		{
			dlog_callback("?", "?", 0, "", buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with file and line information. Use like printf().
 */
void dd_log_flf(char *file, int line, char *func, const char *string, ...)
{
	va_list args;
	char buf[512];

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s():%d: %s\n", file, func, line, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s():%d: %s\n", file, func, line, buf);
		}
		if (dlog_callback)
		{
			dlog_callback(file, func, line, "", buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with error-level. Use like printf().
 */
void dd_log_e(const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_ERROR)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s\n", el_string, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s\n", el_string, buf);
		}
		if (dlog_callback)
		{
			dlog_callback("?", "?", 0, el_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with file and line information. Use like printf().
 */
void dd_log_flfe(const char *file, int line, const char *func, const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_ERROR)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s:%s():%d: %s\n", el_string, file, func, line, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s:%s():%d: %s\n", el_string, file, func, line, buf);
		}
		if (dlog_callback)
		{
			dlog_callback(file, func, line, el_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with warning-level. Use like printf().
 */
void dd_log_w(const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_WARNING)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s\n", wl_string, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s\n", wl_string, buf);
		}
		if (dlog_callback)
		{
			dlog_callback("?", "?", 0, wl_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with file and line information. Use like printf().
 */
void dd_log_flfw(const char *file, int line, const char *func, const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_WARNING)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s:%s():%d: %s\n", wl_string, file, func, line, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s:%s():%d: %s\n", wl_string, file, func, line, buf);
		}
		if (dlog_callback)
		{
			dlog_callback(file, func, line, wl_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with info-level. Use like printf().
 */
void dd_log_i(const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_INFO)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s\n", il_string, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s\n", il_string, buf);
		}
		if (dlog_callback)
		{
			dlog_callback("?", "?", 0, il_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with file and line information. Use like printf().
 */
void dd_log_flfi(const char *file, int line, const char *func, const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_INFO)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s:%s():%d: %s\n", il_string, file, func, line, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s:%s():%d: %s\n", il_string, file, func, line, buf);
		}
		if (dlog_callback)
		{
			dlog_callback(file, func, line, il_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with debug-level. Use like printf().
 */
void dd_log_d(const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_DEBUG)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s\n", dl_string, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s\n", dl_string, buf);
		}
		if (dlog_callback)
		{
			dlog_callback("?", "?", 0, dl_string, buf);
		}
	}

	/* End args. */
	va_end(args);
}

/**
 * Print string to LOG with file and line information. Use like printf().
 */
void dd_log_flfd(const char *file, int line, const char *func, const char *string, ...)
{
	va_list args;
	char buf[512];

	/* check log level */
	if (log_level > DD_LOG_DEBUG)
	{
		return;
	}

	/* Get args. */
	va_start(args, string);
	vsnprintf(buf, sizeof(buf), string, args);

	/* Print to log. */
	if (print_enable)
	{
		if (dlog_file)
		{
			fprintf(dlog_file, "%s:%s:%s():%d: %s\n", dl_string, file, func, line, buf);
		}
		if (print_stderr)
		{
			fprintf(stderr, "%s:%s:%s():%d: %s\n", dl_string, file, func, line, buf);
		}
		if (dlog_callback)
		{
			dlog_callback(file, func, line, dl_string, buf);
		}
	}

	va_end(args);
}

/** Flush log. */
void dd_log_flush(void)
{
	if (dlog_file)
	{
		fflush(dlog_file);
	}
}

/** Enable printing to stderr. */
void dd_log_enable_stderr(void)
{
	print_stderr = 1;
}

/** Disable printing to stderr. */
void dd_log_disable_stderr(void)
{
	print_stderr = 0;
}

/** Enable printing to log. */
void dd_log_enable(void)
{
	print_enable = 1;
}

/** Disable printing to log. */
void dd_log_disable(void)
{
	print_enable = 0;
}
