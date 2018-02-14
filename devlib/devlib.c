/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include "devlib.h"


struct devlib *devlib_first = NULL;
struct devlib *devlib_last = NULL;


int devlib_init()
{
	devlib_first = NULL;
	devlib_last = NULL;
	return 0;
}

void devlib_quit()
{
	/** @todo devlib_quit() */
}

int devlib_add_library(const char *lib_name)
{
	int err = 0;
	struct devlib *lib = NULL;
	char *dll_filename = NULL;

	/* some basic allocation */
	DD_SALLOC(lib);
	DD_STRDUP(lib->name, lib_name);

	/* create real dll filename */
#ifdef _WIN32
	err = asprintf(&dll_filename, "devlib-%s.dll", lib_name);
#else
	err = asprintf(&dll_filename, "libdevlib-%s.so", lib_name);
#endif
	IF_ERR(err < 0, -1, "out of memory? could not print library filename");

	/* open dll */
	lib->dll = dll_open(dll_filename);
	IF_ERR(!lib->dll, -1, "unable to open device library: %s (%s)", lib_name, dll_filename);

	/* find function symbols */
	IF_ERR(!(lib->init = dll_sym(lib->dll, "dev_init")), -1, "function dev_init() not found for device library %s", lib_name);
	IF_ERR(!(lib->quit = dll_sym(lib->dll, "dev_quit")), -1, "function dev_quit() not found for device library %s", lib_name);
	IF_ERR(!(lib->scan = dll_sym(lib->dll, "dev_scan")), -1, "function dev_scan() not found for device library %s", lib_name);
	IF_ERR(!(lib->open = dll_sym(lib->dll, "dev_open")), -1, "function dev_open() not found for device library %s", lib_name);
	IF_ERR(!(lib->close = dll_sym(lib->dll, "dev_close")), -1, "function dev_close() not found for device library %s", lib_name);
	IF_ERR(!(lib->reset = dll_sym(lib->dll, "dev_reset")), -1, "function dev_reset() not found for device library %s", lib_name);
	IF_ERR(!(lib->manipulate = dll_sym(lib->dll, "dev_manipulate")), -1, "function dev_manipulate() not found for device library %s", lib_name);
	IF_ERR(!(lib->ioctl = dll_sym(lib->dll, "dev_ioctl")), -1, "function dev_ioctl() not found for device library %s", lib_name);

	IF_ERR(lib->init(), -1, "function init() failed for device library %s", lib_name);
	//IF_ERR(lib->scan() < 0, -1, "function find() failed for device library %s", lib_name);

	/* add library to list */
	LL_APP(devlib_first, devlib_last, lib);
	err = 0;

out_err:
	if (dll_filename) {
		free(dll_filename);
	}
	if (err != 0 && lib) {
		if (lib->name) {
			free(lib->name);
		}
		free(lib);
	}
	return err;
}

char **devlib_scan(const char *lib_name, int *count)
{
	int n = 0, i;
	struct devlib *lib;
	struct ll_str *ss[2] = { NULL, NULL };
	struct ll_str *s;
	char **serials;

	/* find library */
	for (lib = devlib_first; lib; lib = lib->next) {
		if (strcmp(lib->name, lib_name) == 0) {
			break;
		}
	}
	if (!lib) {
		WARNING_MSG("library not found: %s", lib_name);
		return NULL;
	}

	n = lib->scan(ss);
	if (n < 0) {
		WARNING_MSG("error when scanning devices for library: %s", lib_name);
		return NULL;
	}
	if (count) {
		*count = n;
	}

	serials = malloc((n + 1) * sizeof(*serials));
	memset(serials, 0, (n + 1) * sizeof(*serials));
	for (s = ss[0], i = 0; s; s = s->next, i++) {
		serials[i] = s->str;
		free(s);
	}

	return serials;
}

struct device *devlib_open(const char *lib_name, const char *serial)
{
	int err = 0;
	struct device *dev = NULL;
	struct devlib *lib;
	/* find library */
	for (lib = devlib_first; lib; lib = lib->next) {
		if (strcmp(lib->name, lib_name) == 0) {
			break;
		}
	}
	if (!lib) {
		WARNING_MSG("library not found: %s", lib_name);
		return NULL;
	}
	DD_SALLOC(dev);
	DD_STRDUP(dev->serial, serial);
	IF_ERR(lib->open(dev), -1, "Unable to open device %s from library %s", serial, lib_name);
	dev->lib = lib;
	dev->connected = 1;
	dev->open = 1;
	return dev;
out_err:
	if (dev) {
		if (dev->serial) {
			free(dev->serial);
		}
		free(dev);
	}
	return NULL;
}

int devlib_reset(struct device *dev)
{
	return dev->lib->reset(dev);
}

const char *devlib_name(struct device *dev)
{
	return dev->name;
}

const char *devlib_serial(struct device *dev)
{
	return dev->serial;
}

int devlib_type(struct device *dev)
{
	return dev->type;
}

int devlib_manipulate(struct device *dev, struct device *src, uint8_t *data, int count)
{
	int format;
	int stride = 1;

	/* setup */
	if (!src && (dev->type == DEV_TYPE_IO_READ || dev->type == DEV_TYPE_IO_WRITE)) {
		format = dev->io.format;
		switch (format) {
		case DEV_FORMAT_DIGITAL:
			stride = (int)ceil((double)dev->io.channels / 8.0);
			break;
		case DEV_FORMAT_UINT8:
		case DEV_FORMAT_INT8:
			stride = 1;
			break;
		case DEV_FORMAT_UINT16:
		case DEV_FORMAT_INT16:
			stride = 2;
			break;
		case DEV_FORMAT_UINT24:
		case DEV_FORMAT_INT24:
			stride = 3;
			break;
		case DEV_FORMAT_UINT32:
		case DEV_FORMAT_INT32:
			stride = 4;
			break;
		}
	} else if (!src) {
		ERROR_MSG("invalid parameters");
		return -1;
	} else if (src->type == DEV_TYPE_IO_READ) {
		int i;
		int *formats = devlib_ioctl(dev, DEV_IOCTL_GET_FORMATS);
		if (formats == DEV_IOCTL_ERROR || formats == DEV_IOCTL_UNKNOWN) {
			ERROR_MSG("device has no formats");
			return -1;
		}
		/* check that device supports source format */
		format = DEV_FORMAT_NONE;
		for (i = 0; formats[i] != DEV_FORMAT_NONE; i++) {
			if (formats[i] == src->io.format) {
				format = src->io.format;
				break;
			}
		}
		if (format == DEV_FORMAT_NONE) {
			ERROR_MSG("unsupported format");
			return -1;
		}
		/* set stride by format */
		switch (format) {
		case DEV_FORMAT_DIGITAL:
			stride = (int)ceil((double)src->io.channels / 8.0);
			break;
		case DEV_FORMAT_UINT8:
		case DEV_FORMAT_INT8:
			stride = src->io.channels;
			break;
		case DEV_FORMAT_UINT16:
		case DEV_FORMAT_INT16:
			stride = src->io.channels * 2;
			break;
		case DEV_FORMAT_UINT24:
		case DEV_FORMAT_INT24:
			stride = src->io.channels * 3;
			break;
		case DEV_FORMAT_UINT32:
		case DEV_FORMAT_INT32:
			stride = src->io.channels * 4;
			break;
		}
	} else {
		ERROR_MSG("invalid parameters");
		return -1;
	}

	return dev->lib->manipulate(dev, data, count, format, stride);
}

void *devlib_ioctl(struct device *dev, int request, ...)
{
	void *p;
	va_list argv;
	va_start(argv, request);
	p = dev->lib->ioctl(dev, request, argv);
	va_end(argv);
	return p;
}

