#include <QApplication>

#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
#include <glib.h>
#endif

#include "mainwindow.h"

int main (int argc, char** argv) {
	QApplication app(argc, argv);

	app.setOrganizationDomain("purplehatstands.com");
	app.setOrganizationName("Purple Hatstands");
	app.setApplicationName("Feeder");
	
#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// For gnome keyring
	g_set_application_name("Feeder");
#endif
	
	MainWindow win;
	win.showMaximized();
	
	return app.exec();
}
