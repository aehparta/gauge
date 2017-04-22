/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftd2xx.h>
#include <dev.h>
#include <linkedlist.h>

int dev_add(const char *name, const char *serial, int type, int format, int channels);

const long double device_intervals[] = { 0.001, 0.0001, 0.00001, 0.000001, 0.000001, 0.0 };
struct device *device_first = NULL;
struct device *device_last = NULL;


int dev_init()
{
#ifndef _WIN32
	FT_SetVIDPID(0x0403, 0x7777);
#endif
	return 0;
}

int dev_scan()
{
	FT_DEVICE_LIST_INFO_NODE *devices = NULL;
	DWORD count = 0, i;
	int real_count = 0;

	if (FT_CreateDeviceInfoList(&count) != FT_OK)
	{
		return -1;
	}

	devices = (FT_DEVICE_LIST_INFO_NODE *)malloc(sizeof(*devices) * count);
	memset(devices, 0, sizeof(*devices) * count);
	if (FT_GetDeviceInfoList(devices, &count) != FT_OK)
	{
		free(devices);
		return -1;
	}

	for (i = 0; i < count; i++)
	{
		FT_HANDLE device;
		FT_PROGRAM_DATA data;
		// char Manufacturer[32];
		// char ManufacturerId[16];
		// char Description[64];
		// char SerialNumber[16];

		int type = DEV_TYPE_IO_READ;
		int format = DEV_FORMAT_DIGITAL;
		int channels = 8;

		/* detect whether this device is supported one */
		if (FT_Open(i, &device) != FT_OK)
		{
			continue;
		}
		memset(&data, 0, sizeof(data));
		data.Signature1 = 0x00000000;
		data.Signature2 = 0xffffffff;
		data.Version = 0x00000000;
		if (FT_EE_ReadEx(device, &data, NULL, NULL, NULL, NULL) != FT_OK)
		{
			continue;
		}
		FT_Close(device);
		if (data.VendorId != 0x0403 || data.ProductId != 0x7777)
		{
//			continue;
		}

		/* device was detected as supported device */
		if (dev_add(devices[i].Description, devices[i].SerialNumber, type, format, channels) != 0)
		{
			/* error detection if needed */
		}
		real_count++;
	}

	free(devices);

	return real_count;
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
	FT_STATUS status;
	uint32_t sample_rate;

	/* if device is not connected */
	IF_ERR(!dev->connected, -1, "device not connected: %s", dev->serial);
	/* if device is already open */
	IF_ERR(dev->open, -1, "device already open: %s", dev->serial);

	/* calculate sample rate from interval */
	sample_rate = 2e8 / (dev->io.interval * 1e9);

	/**
	 * @todo this is not right yet, fix
	 */
	IF_ERR(sample_rate < 1 || sample_rate > 30000000, -1, "invalid sample interval %lf s for device: %s", dev->io.interval, dev->serial);

	/* open device */
	status = FT_OpenEx((void *)dev->serial, FT_OPEN_BY_SERIAL_NUMBER, &dev->data);
	IF_ERR(status != FT_OK, -1, "FT_OpenEx() = %d\n", (int)status);

	/* set latency??? */
	// FT_SetLatencyTimer(dev->data, 0xff);

	/* set mode */
	status = FT_SetBitMode(dev->data, 0x00, 0x01);
	if (status != FT_OK)
	{
		ERROR_MSG("FT_SetBitMode() = %d\n", (int)status);
		FT_Close(dev->data);
		OUT_ERR(-1);
	}

	/* set sample rate */
	status = FT_SetBaudRate(dev->data, sample_rate);
	if (status != FT_OK)
	{
		ERROR_MSG("FT_SetBaudRate() = %d\n", (int)status);
		FT_Close(dev->data);
		OUT_ERR(-1);
	}

	/* mark device as open */
	dev->open = 1;
	dev->io.timestamp = 0.0;

	return dev;

out_err:
	return NULL;
}

int dev_close(struct device *dev)
{
	/* if device is not open */
	if (!dev->open)
	{
		WARNING_MSG("trying to close device that is not open");
		return -1;
	}

	/* close device */
	dev->open = 0;
	if (FT_Close(dev->data) != FT_OK)
	{
		WARNING_MSG("closing device failed");
		return -1;
	}

	return 0;
}

int dev_reset(struct device *dev)
{
	/* if device is not open */
	if (!dev->open)
	{
		WARNING_MSG("trying to reset device that is not open");
		return -1;
	}

	/* do reset ... */
	if (FT_Purge(dev->data, FT_PURGE_RX | FT_PURGE_TX) != FT_OK)
	{
		WARNING_MSG("purging device buffers failed");
		return -1;
	}
	dev->io.timestamp = 0.0;

	return 0;
}

int dev_manipulate(struct device *dev, uint8_t *data, int count, int format, int stride)
{
	int err = 0;
	DWORD bytes_returned;
	FT_STATUS status;

	/* read data from device */
	status = FT_Read(dev->data, data, count, &bytes_returned);
	IF_ERR(status != FT_OK, -1, "unable to read from device (%d)", status);

	/* update timestamp */
	dev->io.timestamp += dev->io.interval * count;

out_err:
	return err;
}

void *dev_ioctl(struct device *dev, int request, va_list argv)
{
	if (request == DEV_IOCTL_GET_INTERVAL)
	{
		return (void *)&dev->io.interval;
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
			dev->connected = 1;
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

	LL_APP(device_first, device_last, dev);

	return 0;
}

