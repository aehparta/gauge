#include <QDebug>
#include <devlib.h>
#include <devlib_process.h>
#include "Devices.h"
#include "Device.h"

Devices::Devices(QObject *parent) : QObject(parent)
{
	devlib_init();
	devlib_process_init();
	devlib_add("test");
	devlib_add("ftd2xx");

	pDuration = 1.0;

//	Device *device;

//	device = new Device(this);
//	pDevices.append(device);
//	device = new Device(this);
//	pDevices.append(device);
}

QList<QObject *> Devices::getDevices()
{
	return pDevices;
}

bool Devices::scan()
{
	int c = devlib_scan();
	if (c < 0)
	{
		qDebug() << "scan failed";
		return false;
	}

	struct device **devices = devlib_list(&c);
	qDebug() << "devices found on scan" << c;
	pDevices.clear();
	for (int i = 0; i < c; i++)
	{
		Device *device = new Device(devices[i], this);
		qDebug() << "new device" << device->getLabel();
		connect(device, SIGNAL(dataDone()), this, SLOT(dataDone()), Qt::UniqueConnection);
		pDevices.append(device);
	}
	emit changedDevices();

	return true;
}

void Devices::triggerAll()
{
	for (int i = 0; i < pDevices.count(); i++)
	{
		Device *device = (Device *)pDevices[i];
		if (device->getGlDataBufferCount() > 0)
		{
			device->setGlDataBufferCount(0);
		}
	}
	devlib_process_trigger(NULL);
}

double Devices::getDuration() const
{
	return pDuration;
}

void Devices::setDuration(const double value)
{
	if (pDuration != value && value > 1e-6)
	{
		pDuration = value;
		emit changedDuration();
	}
}

bool Devices::isSampling()
{
	return devlib_process_sampling(NULL) ? true : false;
}

void Devices::setSampling(const bool value)
{
	if (value == isSampling())
	{
		return;
	}
	if (value)
	{
		qDebug() << "start";
		devlib_process_start(NULL);
	}
	else
	{
		qDebug() << "pause";
		devlib_process_pause(NULL);
	}
	emit changedSampling();
}

void Devices::dataDone()
{
	if (!devlib_process_sampling(NULL))
	{
		qDebug() << "all done";
		emit changedSampling();
	}
}

