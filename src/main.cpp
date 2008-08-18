#include <QApplication>

#include "mainwindow.h"

#include "database.h"
#include "atomfeed.h"
#include <QFile>

int main (int argc, char** argv) {
	QApplication app(argc, argv);

	app.setApplicationName("Feeder");

	MainWindow win;
	win.show();
	
	return app.exec();
}
