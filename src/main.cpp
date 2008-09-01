#include <QApplication>

#include "mainwindow.h"

#include "longcatbar.h"

int main (int argc, char** argv) {
	QApplication app(argc, argv);

	app.setOrganizationDomain("purplehatstands.com");
	app.setOrganizationName("Purple Hatstands");
	app.setApplicationName("Feeder");
	
	MainWindow win;
	win.showMaximized();
	
	return app.exec();
}
