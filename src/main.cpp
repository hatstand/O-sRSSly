#include <QApplication>
#include <QtDebug>

#include "atomfeed.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	
	QFuture<AtomFeed*> future(AtomFeed::parse("test.atom"));
	AtomFeed* feed = future.result();
	
	qDebug() << feed->title();
	foreach (const AtomEntry& e, feed->entries())
		qDebug() << e;
	
	return 0;
}
