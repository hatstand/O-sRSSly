#include <QApplication>
#include <QtDebug>

#include "atomfeed.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	
	AtomFeed* feed = new AtomFeed;
	feed->parse("test.atom");
	
	qDebug() << feed->title();
	foreach (const AtomEntry& e, feed->entries())
		qDebug() << e;
	
	return 0;
}
