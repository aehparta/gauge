#ifndef SIGNALCANVAS_H
#define SIGNALCANVAS_H

#include <QObject>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>

#include "Devices.h"
#include "Device.h"
#include "Channel.h"

class SignalCanvas : public QQuickItem, protected QOpenGLFunctions
{
	Q_OBJECT

	Q_PROPERTY(double intervalX READ getIntervalX WRITE setIntervalX NOTIFY changedIntervalX)
	Q_PROPERTY(double offsetX READ getOffsetX WRITE setOffsetX NOTIFY changedOffsetX)
	Q_PROPERTY(QPointF cursor READ getCursor WRITE setCursor NOTIFY changedCursor)
	Q_PROPERTY(QObject *devices READ getDevices WRITE setDevices NOTIFY changedDevices)
	Q_PROPERTY(QList<QObject *> channels READ getChannels NOTIFY changedChannels)
	Q_PROPERTY(bool hasValidOpenGl READ hasValidOpenGl NOTIFY changedHasValidOpenGl)

public:
	SignalCanvas();
	~SignalCanvas();

	const QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());
	void setSetting(const QString &key, const QVariant &value);
	void setSetting(const QString &key, const QString &value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const int value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const double value) { setSetting(key, QVariant(value)); }
	void setSetting(const QString &key, const bool value) { setSetting(key, QVariant(value)); }

	double getIntervalX() const;
	void setIntervalX(const double value);
	double getOffsetX() const;
	void setOffsetX(const double value);
	QPointF getCursor() const;
	void setCursor(const QPointF value);

	bool hasValidOpenGl() const;

	QObject *getDevices() const;
	void setDevices(QObject *devices);

	QList<QObject *> getChannels();

	Q_INVOKABLE void requestPaint(bool doTests = false);

signals:
	void changedIntervalX();
	void changedOffsetX();
	void changedCursor();
	void changedDevices();
	void changedChannels();
	void changedHasValidOpenGl();

public slots:
	void prePaint();
	void paint();
	void dataUpdated();

private slots:
	void handleWindowChanged(QQuickWindow *window);

private:
	void initialize();
	void paintGrid();
	void paintChannel(Channel *channel, bool test);
	void updateData(Device *device);

	int pIndex;
	double pIntervalX;
	double pOffsetX;

	QPointF pCursor;
	QPointF pCursorWindow;
	QPointF pCursorCartesian;

	bool pHasValidOpenGl;

	QOpenGLShaderProgram *shaderDigitalSimple;
	QOpenGLShaderProgram *shaderAnalogSimple;
	QOpenGLShaderProgram *shaderGrid;
	GLuint glGridBufferId;
	GLuint glVao;
	GLuint glQueryId;

	Devices *pDevices;

	bool pDoTests;
};

#endif // SIGNALCANVAS_H
