#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include <QApplication>
#include <QDebug>

#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#include <glib.h>
#endif

#ifdef Q_OS_UNIX
#include "sigsegv.h"
#endif

#include "atomfeed.h"
#include "feedsmodel.h"
#include "keychain.h"
#include "seriousapp.h"
#include "settings.h"
#include "spawn/spawn.h"
#include "spawn/manager.h"
#include "mainwindow.h"

#include "../config.h"

int main(int argc, char** argv) {
	SeriousApp app(argc, argv);

#ifdef USE_SPAWN
	Q_INIT_RESOURCE(spawn);
	
	QString server(Spawn::Manager::serverName());
	if (!server.isNull()) {
#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
		setup_sigsegv();
#endif // defined(Q_OS_UNIX) && defined(QT_DEBUG)
		Spawn::Spawn spawn(server);
		return app.exec();
	}
#endif // USE_SPAWN

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
	app.setWindowIcon(QIcon(":/logo.png"));

	qRegisterMetaType<VoidFunction>("VoidFunction");
	qRegisterMetaType<AtomFeed>("AtomFeed");

	MainWindow win;
	QByteArray geometry = Settings::instance()->geometry();
	if (!geometry.isEmpty()) {
		win.restoreGeometry(geometry);
		win.show();
	} else {
		win.showMaximized();
	}
	
	return app.exec();
}
