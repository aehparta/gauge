#ifndef DEVICES_H
#define DEVICES_H

#include <QObject>
#include <devlib.h>

class Devices : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<QObject *> devices READ getDevices NOTIFY changedDevices)
	Q_PROPERTY(double duration READ getDuration WRITE setDuration NOTIFY changedDuration)
	Q_PROPERTY(bool sampling READ isSampling WRITE setSampling NOTIFY changedSampling)

public:
	Devices(QObject *parent = 0);
	QList<QObject *> getDevices();
	Q_INVOKABLE bool scan();
	Q_INVOKABLE void triggerAll();

	double getDuration() const;
	void setDuration(const double value);

	bool isSampling();
	void setSampling(const bool value);

signals:
	void trigger();
	void changedDevices();
	void changedDuration();
	void changedSampling();

public slots:
	void dataDone();

private:
	QList<QObject *> pDevices;
	double pDuration;
};

#endif // DEVICES_H
