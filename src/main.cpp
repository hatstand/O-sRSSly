#include <QApplication>

#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#include <glib.h>
#endif

#ifdef Q_OS_DARWIN
#include <Carbon/Carbon.h>
#endif

#include "mainwindow.h"

#include "spawn/manager.h"
#include "spawn/view.h"
#include "spawn/spawn.h"
#include <QDebug>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	
	QString server(Spawn::Manager::serverName());
	if (!server.isNull()) {
		Spawn::Spawn spawn(server);
		return app.exec();
	}
	
	Spawn::Manager manager;
	Spawn::View view(&manager);
	view.resize(1024, 768);
	view.show();
	QObject::connect(&view, SIGNAL(titleChanged(const QString&)), &view, SLOT(setWindowTitle( const QString& )));
	
	return app.exec();

	app.setOrganizationDomain("purplehatstands.com");
	app.setOrganizationName("Purple Hatstands");
	app.setApplicationName("Feeder");
	
#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// For gnome keyring
	g_set_application_name("Feeder");
#endif

#ifdef Q_OS_DARWIN
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#endif
	
	MainWindow win;
	win.showMaximized();
	
	return app.exec();
}
