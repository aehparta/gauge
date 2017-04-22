/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dev.h>
#include <linkedlist.h>

const double device_frequency = 1000.0; /* Hz */
const long double device_intervals[] = { 0.001, 0.0001, 0.00001, 0.000001, 0.0 };
struct device *device_first = NULL;
struct device *device_last = NULL;


int dev_init()
{
	return 0;
}

void dev_quit()
{
}

int dev_scan(void *lib)
{
	devlib_add_device(lib, "Digital 1 kHz", "ES1111-0", DEV_TYPE_IO_READ, DEV_FORMAT_DIGITAL, 8);
	devlib_add_device(lib, "Sine 1 kHz", "ES1111-1", DEV_TYPE_IO_READ, DEV_FORMAT_INT8, 1);
	devlib_add_device(lib, "Sawtooth 1 kHz", "ES1111-2", DEV_TYPE_IO_READ, DEV_FORMAT_INT8, 1);
	return 3;
}

int dev_open(struct device *dev)
{
	/* "open" device */
	dev->io.timestamp = -1.0;
	dev->data = malloc(sizeof(double));
	*((double *)dev->data) = 0;
	return 0;
}

int dev_close(struct device *dev)
{
	/* release resources */
	free(dev->data);
	return 0;
}

int dev_reset(struct device *dev)
{
	dev->io.timestamp = -1.0;
	*((double *)dev->data) = 0;
	return 0;
}

int dev_manipulate(struct device *dev, uint8_t *data, int count, int format, int stride)
{
	int i;

	/* wait until there actually should be samples available:
	 * this should not be needed with real hardware as real hardware
	 * should feed samples in timely manner i.e. in sync
	 */
	do
	{
#ifndef _WIN32
		/* only in linux, window has a timer that is not ACCURATE */
		os_sleep(0.000001);
#endif
	}
	while (dev->io.timestamp > os_time());

	if (strcmp(dev->serial, "ES1111-0") == 0)
	{
		double value = *((double *)dev->data);
		for (i = 0; i < count; i++)
		{
			int x = (int)(value * device_frequency * dev->io.interval);
			if (x > 255)
			{
				value = 0;
				x = 0;
			}
			*data = (uint8_t)x;
			data += stride;
			value += 2.0;
		}
		*((double *)dev->data) = value;
	}
	else if (strcmp(dev->serial, "ES1111-1") == 0)
	{
		double fraction = *((double *)dev->data);
		double integral;
		int8_t *samples = (int8_t *)data;
		for (i = 0; i < count; i++)
		{
			*samples = (int8_t)(sin(2 * M_PI * fraction) * 127.0);
			samples += stride;
			fraction += device_frequency * dev->io.interval;
		}
		*((double *)dev->data) = modf(fraction, &integral);
	}
	else if (strcmp(dev->serial, "ES1111-2") == 0)
	{
		double value = *((double *)dev->data);
		int8_t *samples = (int8_t *)data;
		for (i = 0; i < count; i++)
		{
			*samples = (int8_t)((int)value - 128);
			samples += stride;
			value += 256.0 * device_frequency * dev->io.interval;
		}
		*((double *)dev->data) = value;
	}

	/* update timestamp */
	if (dev->io.timestamp < 0.0)
	{
		/*
		 * normally you should only add, but this is a test device
		 * so it needs to start from current time to able to sync.
		 */
		dev->io.timestamp = os_time();
	}
	else
	{
		dev->io.timestamp += dev->io.interval * count;
	}

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
		int i;
		long double interval = va_arg(argv, long double);

		/* check interval */
		for (i = 0; device_intervals[i] > 0.0; i++)
		{
			if (device_intervals[i] != interval)
			{
				continue;
			}
			dev->io.interval = interval;
			return DEV_IOCTL_OK;
		}

		return DEV_IOCTL_ERROR;
	}
	else if (request == DEV_IOCTL_INTERVALS)
	{
		return (void *)device_intervals;
	}

	return DEV_IOCTL_UNKNOWN;
}


