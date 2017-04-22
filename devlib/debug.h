/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

/** Log levels */
enum DD_LOG_LEVELS
{
	DD_LOG_DEBUG = 0,
	DD_LOG_INFO,
	DD_LOG_WARNING,
	DD_LOG_ERROR,
};

/* allocate memory for struct, use IF_ERR() to report errors and set memory to zero */
#define DD_SALLOC(p_item) \
do { \
    p_item = malloc(sizeof(*p_item)); \
    IF_ERR(p_item == NULL, -1, "malloc() failed"); \
    memset(p_item, 0, sizeof(*p_item)); \
} while (0)


/* allocate memory, use IF_ERR() to report errors and set memory to zero */
#define DD_ALLOC(p_item, size) \
do { \
    p_item = malloc((size)); \
    IF_ERR(p_item == NULL, -1, "malloc() failed"); \
    memset(p_item, 0, (size)); \
} while (0)


/* allocate memory for string and copy given string to it, use IF_ERR() to report errors */
#define DD_STRDUP(p_item, p_dup) \
do { \
    p_item = malloc(strlen(p_dup) + 1); \
    IF_ERR(p_item == NULL, -1, "strdup failed on malloc()"); \
    memset(p_item, 0, strlen(p_dup) + 1); \
    strcpy(p_item, p_dup); \
} while (0)


/** Macro definition for DLogflf to ease "__LINE__, __FILE__,__FUNCTION__" print. */
#define _FLF                __FILE__,__LINE__,__FUNCTION__

/** Macro definition. */
#define IF_ER(errval, retval) \
do { \
    if ((errval) != 0) { \
        err = retval; \
        goto out_err; \
    } \
} while (0)

/** Macro definition. */
#define OUT_ERR(retval) do { err = retval; goto out_err; } while (0)

/** Macro definition. */
#ifdef _DEBUG
#define IF_ERR(errval, retval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_flfe(_FLF, __VA_ARGS__); \
        err = retval; \
        goto out_err; \
    } \
} while (0)
#define DEBUG_MSG(...) dd_log_flfd(_FLF, __VA_ARGS__);
#define INFO_MSG(...) dd_log_flfi(_FLF, __VA_ARGS__);
#define ERROR_MSG(...) dd_log_flfe(_FLF, __VA_ARGS__);
#define WARNING_MSG(...) dd_log_flfw(_FLF, __VA_ARGS__);
#define IF_EMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_flfe(_FLF, __VA_ARGS__); \
    } \
} while (0)
#define IF_IMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_flfi(_FLF, __VA_ARGS__); \
    } \
} while (0)
#define IF_WMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_flfw(_FLF, __VA_ARGS__); \
    } \
} while (0)
#define IF_DMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_flfd(_FLF, __VA_ARGS__); \
    } \
} while (0)

#else
#define IF_ERR(errval, retval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_e(__VA_ARGS__); \
        err = retval; \
        goto out_err; \
    } \
} while (0)
#define DEBUG_MSG(...) dd_log_d(__VA_ARGS__);
#define INFO_MSG(...) dd_log_i(__VA_ARGS__);
#define ERROR_MSG(...) dd_log_e(__VA_ARGS__);
#define WARNING_MSG(...) dd_log_w(__VA_ARGS__);
#define IF_EMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_e(__VA_ARGS__); \
    } \
} while (0)
#define IF_IMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_i(__VA_ARGS__); \
    } \
} while (0)
#define IF_WMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_w(__VA_ARGS__); \
    } \
} while (0)
#define IF_DMSG(errval, ...) \
do { \
    if ((errval) != 0) { \
        dd_log_d(__VA_ARGS__); \
    } \
} while (0)
#endif

#define _DEBUG_MSG(...)
#define _INFO_MSG(...)
#define _ERROR_MSG(...)
#define _WARNING_MSG(...)

#ifdef __cplusplus
extern "C" {
#endif

void DLLEXP dd_log_init(char *);
void DLLEXP dd_log_init_syslog(char *);
void DLLEXP dd_log_quit(void);
void DLLEXP dd_log_init_callback(void (*callback)(const char *file, const char *function, int line, const char *type, const char *message));
void DLLEXP dd_log_set_level(int level);

void DLLEXP dd_log(const char *string, ...);
void DLLEXP dd_log_flf(char *, int, char *, const char *, ...);

void DLLEXP dd_log_e(const char *string, ...);
void DLLEXP dd_log_flfe(const char *, int, const char *, const char *, ...);
void DLLEXP dd_log_w(const char *string, ...);
void DLLEXP dd_log_flfw(const char *, int, const char *, const char *, ...);
void DLLEXP dd_log_i(const char *string, ...);
void DLLEXP dd_log_flfi(const char *, int, const char *, const char *, ...);
void DLLEXP dd_log_d(const char *string, ...);
void DLLEXP dd_log_flfd(const char *, int, const char *, const char *, ...);

void DLLEXP dd_log_flush(void);

void DLLEXP dd_log_enable_stderr(void);
void DLLEXP dd_log_disable_stderr(void);
void DLLEXP dd_log_enable(void);
void DLLEXP dd_log_disable(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H__ */

