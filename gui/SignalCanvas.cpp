#include <QSettings>
#include <QDebug>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_3_Compatibility>
#include "SignalCanvas.h"

SignalCanvas::SignalCanvas()
{
	pIndex = 0.0;
	pIntervalX = getSetting("pIntervalX", 1.0).toDouble();
	pOffsetX = 0.0;
	pDevices = NULL;
	pCursor.setX(0.0);
	pCursor.setY(0.0);
	pCursorWindow.setX(0.0);
	pCursorWindow.setY(0.0);
	pCursorCartesian.setX(-1.0);
	pCursorCartesian.setY(-1.0);
	pDoTests = false;

	shaderAnalogSimple = NULL;
	shaderDigitalSimple = NULL;
	shaderGrid = NULL;

	pHasValidOpenGl = false;

	connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

SignalCanvas::~SignalCanvas()
{
	delete shaderDigitalSimple;
	delete shaderAnalogSimple;
	delete shaderGrid;
}

const QVariant SignalCanvas::getSetting(const QString &key, const QVariant &defaultValue)
{
	QSettings settings;
	/* crete full key */
	QString fullKey = "canvas/" + QString::number(pIndex) + "/" + key;
	return settings.value(fullKey, defaultValue);
}

void SignalCanvas::setSetting(const QString &key, const QVariant &value)
{
	QSettings settings;
	/* crete full key */
	QString fullKey = "canvas/" + QString::number(pIndex) + "/" + key;
	settings.setValue(fullKey, value);
}

double SignalCanvas::getIntervalX() const
{
	return pIntervalX;
}

void SignalCanvas::setIntervalX(const double value)
{
	if (pIntervalX != value)
	{
		pIntervalX = value;
		setSetting("pIntervalX", pIntervalX);
		emit changedIntervalX();
	}
}

double SignalCanvas::getOffsetX() const
{
	return pOffsetX;
}

void SignalCanvas::setOffsetX(const double value)
{
	if (pOffsetX != value)
	{
		pOffsetX = value;
		emit changedOffsetX();
	}
}

QPointF SignalCanvas::getCursor() const
{
	return pCursor;
}

void SignalCanvas::setCursor(const QPointF value)
{
	if (pCursor != value)
	{
		/* convert from top-left to bottom-left coordinates (system -> opengl) */
		QPointF opengl_window_space(value.x(), height() - value.y() - 1);
		pCursorWindow = opengl_window_space;
		/* convert to cartesian coordinates */
		pCursorCartesian.setX(opengl_window_space.x() * 2.0 / width() - 1.0);
		pCursorCartesian.setY(opengl_window_space.y() * 2.0 / height() - 1.0);

		/* set unmodified value itself */
		pCursor = value;
		emit changedCursor();
	}
}

bool SignalCanvas::hasValidOpenGl() const
{
	return pHasValidOpenGl;
}

QObject *SignalCanvas::getDevices() const
{
	return pDevices;
}

void SignalCanvas::setDevices(QObject *devices)
{
	if (pDevices != devices)
	{
		pDevices = (Devices *)devices;
		QList<QObject *> objects = pDevices->getDevices();
		for (int i = 0; i < objects.count(); i++)
		{
			Device *device = (Device *)objects[i];
			connect(device, SIGNAL(dataReceived()), this, SLOT(dataUpdated()), Qt::UniqueConnection);
		}
		emit changedDevices();
	}
}

QList<QObject *> SignalCanvas::getChannels()
{
	QList<QObject *> channels;
	QList<QObject *> objects = pDevices->getDevices();
	for (int i = 0; i < objects.count(); i++)
	{
		Device *device = (Device *)objects[i];
		channels += device->getChannels();
	}
	return channels;
}

void SignalCanvas::handleWindowChanged(QQuickWindow *window)
{
	qDebug() << "window changed";
	if (window)
	{
		//connect(window, SIGNAL(beforeRendering()), this, SLOT(paintBefore()), Qt::DirectConnection);
		connect(window, SIGNAL(beforeSynchronizing()), this, SLOT(prePaint()), Qt::DirectConnection);
		connect(window, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
		window->setClearBeforeRendering(false);
	}
}

void SignalCanvas::initialize()
{
	static bool once = false;
	if (once)
	{
		return;
	}
	once = true;

	initializeOpenGLFunctions();

	QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
	qDebug() << "opengl version" << QOpenGLContext::currentContext()->format().version();
	qDebug() << "opengl profile" << QOpenGLContext::currentContext()->format().profile();
	bool result;

	/* generate single query id, mostly used by collision tests in fragment shaders */
	f->glGenQueries(1, &glQueryId);

	/* generate default vertex array objects (needed in core mode) */
	f->glGenVertexArrays(1, &glVao);

	/* create grid shader (basicly simple line drawing shader) */
	shaderGrid = new QOpenGLShaderProgram();
	result = shaderGrid->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/grid.vert");
	if (!result) qDebug() << shaderGrid->log();
	result = shaderGrid->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/grid.frag");
	if (!result) qDebug() << shaderGrid->log();

	shaderGrid->bindAttributeLocation("point", 0);

	result = shaderGrid->link();
	if (!result) qDebug() << shaderGrid->log();

	glGenBuffers(1, &glGridBufferId);

	if (QOpenGLContext::currentContext()->format().version() < qMakePair(3, 3))
    {
        /* missing opengl 3.0 or greater */
		pHasValidOpenGl = false;
		emit changedHasValidOpenGl();
        return;
    }

	/* opengl version accepted */
	pHasValidOpenGl = true;
	emit changedHasValidOpenGl();

	/* create simple digital data shader */
	shaderDigitalSimple = new QOpenGLShaderProgram();
	result = shaderDigitalSimple->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/digital_simple_vertex.vert");
	if (!result) qDebug() << shaderDigitalSimple->log();
    //result = shaderDigitalSimple->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/digital_simple_geometry.vert");
    //if (!result) qDebug() << shaderDigitalSimple->log();
	result = shaderDigitalSimple->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/digital_simple_fragment.frag");
	if (!result) qDebug() << shaderDigitalSimple->log();

	shaderDigitalSimple->bindAttributeLocation("byte_even", 0);
	shaderDigitalSimple->bindAttributeLocation("byte_odd", 1);

	result = shaderDigitalSimple->link();
	if (!result) qDebug() << shaderDigitalSimple->log();

	/* create simple analog data shader */
	shaderAnalogSimple = new QOpenGLShaderProgram();
	result = shaderAnalogSimple->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/analog_simple_vertex.vert");
	if (!result) qDebug() << shaderAnalogSimple->log();
	result = shaderAnalogSimple->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/analog_simple_fragment.frag");
	if (!result) qDebug() << shaderAnalogSimple->log();

	shaderAnalogSimple->bindAttributeLocation("data", 0);

	result = shaderAnalogSimple->link();
	if (!result) qDebug() << shaderAnalogSimple->log();
}

void SignalCanvas::prePaint()
{
	if (pDoTests)
	{
		paint();
	}
}

void SignalCanvas::paint()
{
	if (!isVisible())
	{
		return;
	}

	/* this is done actually only once (see the code above) */
	initialize();

	/* reset opengl state */
	window()->resetOpenGLState();

	/* need to enable vertex array object when using core mode */
	QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
	f->glBindVertexArray(glVao);

	/* map drawable area coordinates to opengl window coordinates */
	QPointF windowCoordinates = mapToScene(QPointF(0, height()));

	/* setup drawable area */
	glViewport(windowCoordinates.x(), window()->height() - windowCoordinates.y(), width(), height());

	if (pDoTests)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.3, 0.4, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
		glScissor(windowCoordinates.x(), window()->height() - windowCoordinates.y(), width(), height());
		glClearColor(0.0, 0.1, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		paintGrid();
		glEnable(GL_PROGRAM_POINT_SIZE);
	}

	if (pDevices)
	{
		QList<QObject *> devices = pDevices->getDevices();
		for (int i = 0; i < devices.length(); i++)
		{
			Device *device = (Device *)devices.at(i);
			updateData(device);
			QList<QObject *> channels = device->getChannels();
			for (int j = 0; j < channels.length(); j++)
			{
				Channel *channel = (Channel *)channels.at(j);
				if (channel->isEnabled() && channel->isVisible())
				{
					paintChannel(channel, pDoTests);
				}
			}
		}
	}

	/* reset tests */
	pDoTests = false;
	/* reset opengl state */
	window()->resetOpenGLState();
}

void SignalCanvas::requestPaint(bool doTests)
{
	pDoTests = doTests;
	window()->update();
}

void SignalCanvas::paintGrid()
{
	glBindBuffer(GL_ARRAY_BUFFER, glGridBufferId);

	static bool once = false;
	if (!once)
	{
		GLfloat vertices[] = {
			/* horizontal main crosshair line */
			0.0, -1.0, 0.0, 1.0,
			/* vertical main crosshair line */
			-1.0, 0.0, 1.0, 0.0,
			/* horizontal grid */
			0.75, -1.0, 0.75, 1.0,
			0.5, -1.0, 0.5, 1.0,
			0.25, -1.0, 0.25, 1.0,
			-0.25, -1.0, -0.25, 1.0,
			-0.5, -1.0, -0.5, 1.0,
			-0.75, -1.0, -0.75, 1.0,
			/* vertical grid */
			-1.0, 0.75, 1.0, 0.75,
			-1.0, 0.5, 1.0, 0.5,
			-1.0, 0.25, 1.0, 0.25,
			-1.0, -0.25, 1.0, -0.25,
			-1.0, -0.5, 1.0, -0.5,
			-1.0, -0.75, 1.0, -0.75,
		};
		/* upload data (maybe do this in initialization someday?) */
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		once = true;
	}

	glLineWidth(1.0);

	shaderGrid->bind();
	shaderGrid->enableAttributeArray(0);

	/* draw grid */
	shaderGrid->setUniformValue("color", QColor(0x20, 0x40, 0x20, 0xff));
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_LINES, 4, 24);

	/* draw crosshair */
	shaderGrid->setUniformValue("color", QColor(0x70, 0x90, 0x70, 0xff));
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_LINES, 0, 4);

	shaderGrid->disableAttributeArray(0);
	shaderGrid->release();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SignalCanvas::paintChannel(Channel *channel, bool test)
{
	Device *device = (Device *)channel->parent();
	QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
	int count = device->getGlDataBufferCount();

	/* if buffer is invalid or has no data, do not try to draw */
	if (count < 1)
	{
		return;
	}

	/* map drawable area coordinates to opengl window coordinates */
	QPointF windowCoordinates = mapToScene(QPointF(0, height()));

	glBindBuffer(GL_ARRAY_BUFFER, device->getGlDataBufferId());

	if (channel->isDigital() && shaderDigitalSimple)
	{
		shaderDigitalSimple->bind();
		shaderDigitalSimple->enableAttributeArray(0);
		shaderDigitalSimple->enableAttributeArray(1);

		shaderDigitalSimple->setUniformValue("cursor",
											 pCursorCartesian.x(),
											 pCursorCartesian.y(),
											 pCursorWindow.x() + windowCoordinates.x(),
											 pCursorWindow.y() + (window()->height() - windowCoordinates.y()));
		shaderDigitalSimple->setUniformValue("color", channel->getColor());
		shaderDigitalSimple->setUniformValue("mask", (uint32_t)(1 << channel->getIndex()));
		shaderDigitalSimple->setUniformValue("scale", pOffsetX, channel->getY(), device->getInterval() / pIntervalX, channel->getScaleY());

		/* do not normalize digital data */
		glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void *)0);
		glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, (void *)1);

		/* check if this channel pixels are near mouse */
		if (test)
		{
			GLuint queryResult = 0;
			f->glBeginQuery(GL_ANY_SAMPLES_PASSED, glQueryId);
			shaderDigitalSimple->setUniformValue("test", true);
			glDrawArrays(GL_LINE_STRIP, 0, count);
			f->glEndQuery(GL_ANY_SAMPLES_PASSED);
			f->glGetQueryObjectuiv(glQueryId, GL_QUERY_RESULT, &queryResult);
			/* check result */
			if (queryResult)
			{
				channel->enableSelecting();
			}
			else
			{
				channel->disableSelecting();
			}
		}
		else
		{
			/* render digital signal */
			shaderDigitalSimple->setUniformValue("test", false);
			if (channel->isSelected())
			{
				f->glDrawArraysInstanced(GL_LINE_STRIP, 0, count, 20);
			}
			else
			{
				glDrawArrays(GL_LINE_STRIP, 0, count);
			}
		}

		shaderDigitalSimple->disableAttributeArray(0);
		shaderDigitalSimple->disableAttributeArray(1);
		shaderDigitalSimple->release();
	}
	else if (!channel->isDigital() && shaderAnalogSimple)
	{
		shaderAnalogSimple->bind();
		shaderAnalogSimple->enableAttributeArray(0);

		shaderAnalogSimple->setUniformValue("cursor",
											pCursorCartesian.x(),
											pCursorCartesian.y(),
											pCursorWindow.x() + windowCoordinates.x(),
											pCursorWindow.y() + (window()->height() - windowCoordinates.y()));
		shaderAnalogSimple->setUniformValue("color", channel->getColor());
		shaderAnalogSimple->setUniformValue("scale", pOffsetX, channel->getY(), device->getInterval() / pIntervalX, channel->getScaleY() / 2);

		/* let opengl normalize analog data to [ 1.0 , -1.0 ] automatically */
		glVertexAttribPointer(0, 1, GL_BYTE, GL_TRUE, 0, (void *)0);

		/* check if this channel pixels are near mouse */
		if (test)
		{
			GLuint queryResult = 0;
			f->glBeginQuery(GL_ANY_SAMPLES_PASSED, glQueryId);
			shaderAnalogSimple->setUniformValue("test", true);
			glDrawArrays(GL_LINE_STRIP, 0, count);
			f->glEndQuery(GL_ANY_SAMPLES_PASSED);
			f->glGetQueryObjectuiv(glQueryId, GL_QUERY_RESULT, &queryResult);
			/* check result */
			if (queryResult)
			{
				channel->enableSelecting();
			}
			else
			{
				channel->disableSelecting();
			}
		}
		else
		{
			/* render analog signal */
			shaderAnalogSimple->setUniformValue("test", false);
			if (channel->isSelected())
			{
				f->glDrawArraysInstanced(GL_LINE_STRIP, 0, count, 20);
			}
			else
			{
				glDrawArrays(GL_LINE_STRIP, 0, count);
			}
		}

		shaderAnalogSimple->disableAttributeArray(0);
		shaderAnalogSimple->release();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void SignalCanvas::updateData(Device *device)
{
	struct devlib_process_buffer *buffer_tmp;
	struct devlib_process_buffer *buffer = device->getProcessBuffer();
	if (!buffer)
	{
		/* no new data */
		return;
	}

	int size = device->getGlDataBufferSize();
	int count = device->getGlDataBufferCount();
	if (count >= size)
	{
		/* all data already uploaded, free left-overs */
		devlib_process_buffer_free(buffer);
		return;
	}

	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError());

	/* generate buffers (once) */
	if (count < 0)
	{
		/* create buffer for storing data */
		GLuint id;
		glGenBuffers(1, &id);
		device->setGlDataBufferId(id);
		device->setGlDataBufferCount(0);
		count = 0;
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, device->getGlDataBufferId());
	}

	/* upload data itself */
	for (buffer_tmp = buffer; buffer_tmp && count < size; buffer_tmp = buffer_tmp->next)
	{
		int bytes = buffer_tmp->count * buffer_tmp->bytes_per_sample;
		if ((count + bytes) > size)
		{
			bytes = size - count;
		}
		glBufferSubData(GL_ARRAY_BUFFER, count, bytes, buffer_tmp->data);
		count += bytes;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (count >= size)
	{
		device->setGlDataBufferCount(size);
	}
	else
	{
		device->setGlDataBufferCount(count);
	}

	devlib_process_buffer_free(buffer);
}

void SignalCanvas::dataUpdated()
{
	requestPaint();
}
