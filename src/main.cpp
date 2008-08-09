#include <QApplication>
#include <QtDebug>

#include "atomfeed.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	
	AtomFeed feed("test.atom");
	
	qDebug() << feed;
	
	return 0;
}
