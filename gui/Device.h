#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <devlib.h>

struct indirect_command {
	GLuint count;
	GLuint instance_count;
	GLuint first;
	GLuint base_instance;
};

class Device : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString label READ getLabel WRITE setLabel NOTIFY changedLabel)
	Q_PROPERTY(QString name READ getName)
	Q_PROPERTY(QString serial READ getSerial)

	Q_PROPERTY(double samplingTime READ getSamplingTime WRITE setSamplingTime NOTIFY changedSamplingTime)

	Q_PROPERTY(QList<QObject *> channels READ getChannels NOTIFY changedChannels)

	Q_PROPERTY(bool done READ isDone NOTIFY changedDone)

public:
	Device(struct device *dev, QObject *parent = 0);

	const QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());
	void setSetting(const QString &key, const QVariant &value);
	void setSetting(const QString &key, const QString &value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const int value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const double value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const bool value) { setSetting(key, QVariant(value)); }

	struct device *getLibraryDevice();
	double getInterval();
	bool setInterval(double value);
	double getSamplingTime();
	void setSamplingTime(double value);

	QString getLabel() const;
	void setLabel(const QString &value);
	QString getName() const;
	QString getSerial() const;

	QList<QObject *> getChannels();

	bool isDone();
	bool isTriggered();

	GLuint getGlDataBufferId();
	void setGlDataBufferId(GLuint value);
	int getGlDataBufferSize();
	void setGlDataBufferSize(int value);
	int getGlDataBufferCount();
	void setGlDataBufferCount(int value);
	struct devlib_process_buffer *getProcessBuffer();

signals:
	void changedLabel();
	void changedChannels(); /* no need to call this really, here just to make QML happier */
	void changedSamplingTime();
	void changedDone();

	void dataReceived();
	void dataDone();

public slots:

private:
	QString pLabel;
	double pSamplingTime;

	struct device *libdev;
	struct devlib_process *libprocess;

	QList<QObject *> pChannels;

	GLuint pGlDataBufferId;
	int pGlDataBufferSize;
	int pGlDataBufferCount;

	/*
	 * This is executed in data processing thread separate from Qt.
	 * Must keep as small and quick as possible!
	 */
	static int processHasData(struct devlib_process *process, void *userdata);
	/* Same here. */
	static int processDone(struct devlib_process *process, void *userdata);
};

#endif // DEVICE_H
