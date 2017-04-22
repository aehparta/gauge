/*
 * devlib edge trigger.
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include <dev.h>
#include <linkedlist.h>

struct device *trigger = NULL;

int dev_init()
{
	trigger = malloc(sizeof(*trigger));
	memset(trigger, 0, sizeof(*trigger));

	/* setup device information */
	trigger->name = strdup("Edge");
	trigger->serial = strdup("trigger-edge");
	trigger->type = DEV_TYPE_TRIGGER;
	trigger->connected = 1;

	return 0;
}

int dev_scan()
{
	return 1;
}

struct device *dev_list(int *count)
{
	if (count)
	{
		*count = 1;
	}
	return trigger;
}

void dev_free(struct device *dev)
{
	/** @todo free device resources */
}

struct device *dev_open(struct device *dev)
{
	struct device *clone = NULL;

	clone = malloc(sizeof(*clone));
	memset(clone, 0, sizeof(*clone));

	/* clone */
	clone->name = strdup(dev->name);
	clone->serial = strdup(dev->serial);
	clone->type = dev->type;
	clone->connected = 1;
	clone->open = 1;
	clone->data = 0;

	return clone;
}

int dev_close(struct device *dev)
{
	return 0;
}

int dev_reset(struct device *dev)
{
	return 0;
}

int dev_manipulate(struct device *dev, uint8_t *data, int count, int format, int stride)
{
	return 0;
}

void *dev_ioctl(struct device *dev, int request, va_list argv)
{
	return DEV_IOCTL_UNKNOWN;
}
