#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStringList>

#include <devlib.h>

#include "Devices.h"
#include "SignalCanvas.h"

void log_cb(const char *file, const char *function, int line, const char *type, const char *message)
{
	QString msg;
	msg.sprintf("%s:%s:%s:%d: %s", type, file, function, line, message);
	qDebug() << msg;
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QQmlApplicationEngine app_engine;

	app.setOrganizationName("TLDR");
	app.setOrganizationDomain("scope.tldr.fi");
	app.setApplicationName("Scope");

	/* initialize backend logging */
	dd_log_init_callback(log_cb);

	qmlRegisterType<SignalCanvas>("SignalCanvas", 1, 0, "SignalCanvas");

	QQmlContext *ctxt = app_engine.rootContext();

	Devices devices;
	devices.scan();
	ctxt->setContextProperty("modelDevices", &devices);

	app_engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	return app.exec();
}

