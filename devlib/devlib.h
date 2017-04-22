/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __DEVLIB_H__
#define __DEVLIB_H__

#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "debug.h"
#include "linkedlist.h"
#include "os.h"
#include "dev.h"


#ifdef __cplusplus
extern "C" {
#endif

struct devlib
{
	dll_t dll;
	char *name;

	/* functions in library */
	int (*init)();
	void (*quit)();
	int (*scan)(void *lib);
	int (*open)(void *dev);
	int (*close)(void *dev);
	int (*reset)(void *dev);
	int (*manipulate)(void *dev, uint8_t *data, int count, int format, int stride);

	void *(*ioctl)(void *dev, int request, va_list argv);

	struct devlib *prev;
	struct devlib *next;
};


DLLEXP int devlib_init();
DLLEXP void devlib_quit();
DLLEXP int devlib_add_library(const char *lib_name);
DLLEXP char **devlib_scan(const char *lib_name);
DLLEXP int devlib_device_found(void *p, const char *serial);
DLLEXP struct device *devlib_open(const char *lib_name, const char *serial);
DLLEXP const char *devlib_serial(struct device *dev);
DLLEXP const char *devlib_name(struct device *dev);
DLLEXP int devlib_type(struct device *dev);
DLLEXP int devlib_reset(struct device *dev);
DLLEXP int devlib_manipulate(struct device *dev, struct device *src, uint8_t *data, int count);
DLLEXP void *devlib_ioctl(struct device *dev, int request, ...);


#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */
