#include <QSettings>
#include <QDebug>

#include "Device.h"
#include "Channel.h"


Device::Device(struct device *dev, QObject *parent) : QObject(parent)
{
	bool digital = dev->io.format == DEV_FORMAT_DIGITAL ? true : false;
	libdev = dev;
	pLabel.clear();

	pSamplingTime = 0.5;
	pGlDataBufferId = 0;
	pGlDataBufferCount = -1;
	setInterval(0.000001);

	libprocess = devlib_process_create(dev);
	devlib_process_set_userdata(libprocess, this);
	devlib_process_set_callback_data(libprocess, processHasData);
	devlib_process_set_callback_done(libprocess, processDone);
	//devlib_process_start(libprocess);
	//devlib_process_trigger(libprocess);

	/* create channels */
	for (int i = 0; i < dev->io.channels; i++)
	{
		Channel *channel = new Channel(i, digital, this);
		pChannels.append(channel);
	}
}

const QVariant Device::getSetting(const QString &key, const QVariant &defaultValue)
{
	QSettings settings;
	/* crete full key */
	QString fullKey = "devices/" + getSerial() + "/" + key;
	return settings.value(fullKey, defaultValue);
}

void Device::setSetting(const QString &key, const QVariant &value)
{
	QSettings settings;
	/* crete full key */
	QString fullKey = "devices/" + getSerial() + "/" + key;
	settings.setValue(fullKey, value);
}

struct device *Device::getLibraryDevice()
{
	return libdev;
}

double Device::getInterval()
{
	void *p = devlib_ioctl(libdev, DEV_IOCTL_GET_INTERVAL);
	if (p == DEV_IOCTL_ERROR || p == DEV_IOCTL_UNKNOWN)
	{
		return 0.0;
	}
	return (double)*(long double *)p;
}

bool Device::setInterval(double value)
{
	void *p = devlib_ioctl(libdev, DEV_IOCTL_SET_INTERVAL, (long double)value);
	if (p != DEV_IOCTL_OK)
	{
		pGlDataBufferSize = 0;
		return false;
	}
	pGlDataBufferSize = (int)ceil(pSamplingTime / value);
	return true;
}

double Device::getSamplingTime()
{
	return pSamplingTime;
}

void Device::setSamplingTime(double value)
{
	if (pSamplingTime != value && value > 1e-6 && value < 10.0)
	{
		pSamplingTime = value;
		emit changedSamplingTime();
	}
}

QString Device::getLabel() const
{
	if (pLabel.isEmpty())
	{
		return getSerial() + ": " + getName();
	}
	return pLabel;
}

void Device::setLabel(const QString &value)
{
	if (pLabel != value)
	{
		pLabel = value;
		emit changedLabel();
	}
}

QString Device::getName() const
{
	return devlib_name(libdev);
}

QString Device::getSerial() const
{
	return devlib_serial(libdev);
}

QList<QObject *> Device::getChannels()
{
	return pChannels;
}

bool Device::isDone()
{
	return devlib_process_done(libprocess) ? true : false;
}

bool Device::isTriggered()
{
	//return devlib_process_triggered(libprocess) ? false : true;
	return false;
}

GLuint Device::getGlDataBufferId()
{
	return pGlDataBufferId;
}

void Device::setGlDataBufferId(GLuint value)
{
	pGlDataBufferId = value;
}

int Device::getGlDataBufferSize()
{
	return pGlDataBufferSize;
}

void Device::setGlDataBufferSize(int value)
{
	pGlDataBufferSize = value;
}

int Device::getGlDataBufferCount()
{
	return pGlDataBufferCount;
}

void Device::setGlDataBufferCount(int value)
{
	pGlDataBufferCount = value;
}

struct devlib_process_buffer *Device::getProcessBuffer()
{
	return devlib_process_buffer_get(libprocess);
}

int Device::processHasData(struct devlib_process *process, void *userdata)
{
	Device *device = (Device *)userdata;
	emit device->dataReceived();
	if (device->getGlDataBufferCount() >= device->getGlDataBufferSize())
	{
		qDebug() << device->getSerial() << "almost done";
		return 1;
	}
	return 0;
}

int Device::processDone(struct devlib_process *process, void *userdata)
{
	Device *device = (Device *)userdata;
	emit device->dataDone();
	qDebug() << device->getSerial() << "done";
	return 0;
}
