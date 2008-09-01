#include <QApplication>

#ifdef Q_OS_UNIX
#include <glib.h>
#endif

#include "mainwindow.h"

int main (int argc, char** argv) {
	QApplication app(argc, argv);

	app.setOrganizationDomain("purplehatstands.com");
	app.setOrganizationName("Purple Hatstands");
	app.setApplicationName("Feeder");
	
#ifdef Q_OS_UNIX
	// For gnome keyring
	g_set_application_name("Feeder");
#endif
	
	MainWindow win;
	win.showMaximized();
	
	return app.exec();
}
