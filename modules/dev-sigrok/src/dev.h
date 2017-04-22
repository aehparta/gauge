/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __DEV_H__
#define __DEV_H__

#ifdef _WIN32
#include <windows.h>
#else
#endif
#include <stdint.h>

#ifdef _WIN32
#define uint32_t DWORD
#define uint16_t WORD
#define uint8_t BYTE
#endif


#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#define DEVICE_NAME     "libsigrok compatible devices"

struct device
{
	char *name;
	char *serial;

	/**
	 * Device mode.
	 *  - 0: digital
	 *  - 1: analog, voltage
	 *  - 2: analog, current
	 */
	uint8_t mode;
	/** Last sample start timestamp. */
	int64_t timestamp;

	int connected;
	int open;

	void *lib;

	struct device *prev;
	struct device *next;

	void *data;
};

struct sample
{
	int64_t timestamp;
	int64_t interval;

	uint8_t *data;
	size_t size;

	struct sample *prev;
	struct sample *next;
};

int DLLEXP dev_init();
const char DLLEXP *dev_name();
int DLLEXP dev_find();
struct device DLLEXP *dev_list(int *count);
struct device DLLEXP *dev_open(const char *serial);
int DLLEXP dev_close(struct device *dev);
int DLLEXP dev_reset(struct device *dev);
struct sample DLLEXP *dev_read(struct device *dev);

/* internal calls */
int dev_add(const char *name, const char *serial, uint8_t mode, int channels);


#ifdef __cplusplus
}
#endif

#endif /* __DEV_H__ */
