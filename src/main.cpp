#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include <QApplication>

#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#include <glib.h>
#endif

#include "spawn/spawn.h"
#include "spawn/manager.h"
#include "mainwindow.h"

#include "../config.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	
	Q_INIT_RESOURCE(spawn);
	
	QString server(Spawn::Manager::serverName());
	if (!server.isNull()) {
		Spawn::Spawn spawn(server);
		return app.exec();
	}

	app.setOrganizationDomain("purplehatstands.com");
	app.setOrganizationName("Purple Hatstands");
	app.setApplicationName(TITLE);
	
#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// For gnome keyring
	g_set_application_name(TITLE);
#endif

#ifdef Q_OS_DARWIN
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
#endif
	
	MainWindow win;
	win.showMaximized();
	
	return app.exec();
}
