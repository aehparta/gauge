/*
 * Multicast receiver.
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <dev.h>
#include <linkedlist.h>

int dev_add(const char *, const char *, int, int, int);

struct device *device_first = NULL;
struct device *device_last = NULL;


int dev_init()
{
	return 0;
}

int dev_scan()
{
	DEBUG_MSG("scan for multicast sources");

	return 0;
}

struct device *dev_list(int *count)
{
	struct device *dev = NULL;
	int i = 0;

	LL_COUNT(device_first, device_last, dev, i);

	if (count)
	{
		*count = i;
	}

	return device_first;
}

void dev_free(struct device *dev)
{
	/** @todo free device resources */
}

struct device *dev_open(struct device *dev)
{
	int err = 0;
	/* if device is not connected */
	IF_ERR(!dev->connected, -1, "device not connected: %s", dev->serial);
	/* if device is already open */
	IF_ERR(dev->open, -1, "device already open: %s", dev->serial);
	/* open device */
	return dev;
out_err:
	return NULL;
}

int dev_close(struct device *dev)
{
	/* if device is not open */
	if (!dev->open)
	{
		return -1;
	}

	/* close device */
	dev->open = 0;

	return 0;
}

int dev_reset(struct device *dev)
{
	/* if device is not open */
	if (!dev->open)
	{
		return -1;
	}

	dev->io.timestamp = 0.0;

	return 0;
}

int dev_manipulate(struct device *dev, uint8_t *data, int count, int format, int stride)
{
	int i;

	/* if device is not open */
	if (!dev->open)
	{
		return -1;
	}

	dev->io.timestamp += dev->io.interval * count;

	return 0;
}

void *dev_ioctl(struct device *dev, int request, va_list argv)
{
	if (request == DEV_IOCTL_GET_INTERVAL)
	{
		return &dev->io.interval;
	}
	else if (request == DEV_IOCTL_SET_INTERVAL)
	{
		return DEV_IOCTL_ERROR;
	}
	else if (request == DEV_IOCTL_INTERVALS)
	{
		return DEV_IOCTL_ERROR;
	}

	return DEV_IOCTL_UNKNOWN;
}

/*
 * internal functions
 */

int dev_add(const char *name, const char *serial, int type, int format, int channels)
{
	struct device *dev = NULL;

	/* check if this device already exists */
	for (dev = device_first; dev; dev = dev->next)
	{
		if (strcmp(dev->serial, serial) == 0)
		{
			/* this device already exists */
			return 0;
		}
	}

	dev = malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	/* setup device information */
	dev->name = strdup(name);
	dev->serial = strdup(serial);
	dev->type = type;
	dev->connected = 1;
	dev->open = 0;
	dev->io.format = format;
	dev->io.channels = channels;
	dev->data = malloc(sizeof(double));

	LL_APP(device_first, device_last, dev);

	return 0;
}

