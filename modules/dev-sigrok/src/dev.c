/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <libsigrok/libsigrok.h>
#include "dev.h"
#include "linkedlist.h"


struct sr_context *sr_ctx = NULL;
struct device *device_first = NULL;
struct device *device_last = NULL;


int dev_init()
{
	int err = 0;

	err = sr_init(&sr_ctx);
	if (err != SR_OK)
	{
		return -1;
	}

	return 0;
}

void dev_quit()
{
	sr_exit(sr_ctx);
}

const char *dev_name()
{
	static const char *name = DEVICE_NAME;
	return name;
}

int dev_find()
{
	int i;
	int dev_count = 0;
	struct sr_dev_driver **drivers;
	struct sr_dev_inst *dev;

	drivers = sr_driver_list();
	if (!drivers)
	{
		return 0;
	}

	for (i = 0; drivers[i]; i++)
	{
		GSList *list;
		if (sr_driver_init(sr_ctx, drivers[i]) != SR_OK)
		{
			continue;
		}
		list = sr_driver_scan(drivers[i], NULL);
		for ( ; list ; list = list->next)
		{
			char *name = NULL;
			char *serial = NULL;
			dev = list->data;
			if (dev->vendor && dev->model && dev->version)
			{
				asprintf(&name, "%s %s %s", dev->vendor, dev->model, dev->version);
			}
			else if (dev->vendor && dev->model)
			{
				asprintf(&name, "%s %s", dev->vendor, dev->model);
			}
			else
			{
				asprintf(&name, "%s", dev->vendor);
			}

			asprintf(&serial, "%s-%04d", drivers[i]->name, dev->index);

			dev_add(name, serial, 0, 0);

			free(name);
			free(serial);
		}
	}

	return dev_count;
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

struct device *dev_open(const char *serial)
{
	struct device *dev = NULL;

	/* find device */
	for (dev = device_first; dev; dev = dev->next)
	{
		if (strcmp(dev->serial, serial) == 0)
		{
			break;
		}
	}

	/* if device not found */
	if (!dev)
	{
		return NULL;
	}

	/* if device is not connected */
	if (!dev->connected)
	{
		return NULL;
	}

	/* "open" device */
	dev->open = 1;

	return dev;
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

	/* no reset needed for test devices */
	return 0;
}

struct sample *dev_read(struct device *dev)
{
	// struct timespec tp;
	struct sample *sample = NULL;

	/* if device is not open */
	if (!dev->open)
	{
		return NULL;
	}

	return sample;
}

int dev_add(const char *name, const char *serial, uint8_t mode, int channels)
{
	struct device *dev = NULL;

	/* check if this device already exists */
	for (dev = device_first; dev; dev = dev->next)
	{
		if (strcmp(dev->serial, serial) == 0)
		{
			/* this device already exists */
			dev->connected = 1;
			return 0;
		}
	}

	dev = malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	/* setup device information */
	dev->name = strdup(name);
	dev->serial = strdup(serial);
	dev->mode = mode;
	dev->connected = 1;
	dev->open = 0;

	LL_APP(device_first, device_last, dev);

	return 0;
}

