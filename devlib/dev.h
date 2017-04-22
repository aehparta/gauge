/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __DEV_H__
#define __DEV_H__

#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <stdarg.h>

#include "debug.h"
#include "linkedlist.h"
#include "os.h"
#include "devlib_process.h"


#ifdef __cplusplus
extern "C" {
#endif

struct device_info_io
{
	/** Number of channels in this device. */
	int channels;
	/**
	 * Data format.
	 *
	 * For analog devices the dev_manipulate() data buffer
	 * will have total bytes of (channels * sizeof(format) * size).
	 * So every single sample for every channel will have sizeof(format)
	 * amount of storage available.
	 *
	 * For digital devices every single sample will have a single bit
	 * available. 1-8 channel devices get (1 * size) bytes of data buffer,
	 * 9-16 channel devices get (2 * size) bytes and so on.
	 */
	int format;
	/** Next sample start in seconds. */
	long double timestamp;
	/** Sample interval in seconds. */
	long double interval;
};

/** devlib process specific data for device */
struct device_info_process
{
	struct devlib_process *process;
	thread_t thread;
	int paused;
	/** data buffer */
	struct devlib_process_buffer *buffer_first;
	struct devlib_process_buffer *buffer_last;
	/** buffer lock */
	mutex_t buffer_mutex;
	/* current buffers count */
	int buffers_count;
	/** buffers to keep before trigger */
	int buffers_pre_trigger;
	/** buffers total (pre-trigger buffers included) */
	int buffers_total;
};

struct device
{
	char *name;
	char *serial;
	int connected;
	int open;
	int type;

	struct devlib *lib;

	struct device *prev;
	struct device *next;

	/* device type specific data combined using union */
	union
	{
		struct device_info_io io;
	};

	/* process specific data */
	struct device_info_process process;

	void *data;
};

#ifdef DEVICE_MODULE
/* device module specific stuff */
DLLEXP int dev_init();
DLLEXP void dev_quit();
DLLEXP int dev_scan(void *lib);
DLLEXP int dev_open(struct device *dev);
DLLEXP int dev_close(struct device *dev);
DLLEXP int dev_reset(struct device *dev);
DLLEXP int dev_manipulate(struct device *dev, uint8_t *data, int count, int format, int stride);
DLLEXP void *dev_ioctl(struct device *dev, int request, va_list argv);
#endif

/**
 * Device ioctl definitions.
 */
enum DEV_IOCTLS
{
	/** Get all intervals supported by this device (long double) */
	DEV_IOCTL_INTERVALS,
	/** Get current device interval (long double) */
	DEV_IOCTL_GET_INTERVAL,
	/** Set current device interval (long double) */
	DEV_IOCTL_SET_INTERVAL,
	/** Get supported formats. */
	DEV_IOCTL_GET_FORMATS,
};
/** ioctl generic ok return value */
#define DEV_IOCTL_OK ((void *)0)
/** ioctl request failed */
#define DEV_IOCTL_ERROR ((void *)-1)
/** ioctl request not supported */
#define DEV_IOCTL_UNKNOWN ((void *)-2)

/**
 * Device type definitions.
 */
enum DEV_TYPES
{
	DEV_TYPE_NONE,
	/** IO device that can be read. */
	DEV_TYPE_IO_READ,
	/** IO device that can be written. */
	DEV_TYPE_IO_WRITE,
	/** Trigger. */
	DEV_TYPE_TRIGGER,
	/** Filter. */
	DEV_TYPE_FILTER,
};

/**
 * Data formats.
 */
enum DEV_FORMATS
{
	DEV_FORMAT_NONE,
	/** Digital format. */
	DEV_FORMAT_DIGITAL,
	/* analog formats */
	DEV_FORMAT_UINT8,
	DEV_FORMAT_INT8,
	DEV_FORMAT_UINT16,
	DEV_FORMAT_INT16,
	DEV_FORMAT_UINT24,
	DEV_FORMAT_INT24,
	DEV_FORMAT_UINT32,
	DEV_FORMAT_INT32,
};


#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */
