#ifndef ATOMFEED_H
#define ATOMFEED_H

#include <QString>
#include <QList>
#include <QtConcurrentRun>

class QIODevice;
class QXmlStreamReader;

class AtomEntry
{
public:
	AtomEntry(QXmlStreamReader& s);
	
	QString title;
	QString id;
	QString summary;
	QString content;
};

QDebug operator <<(QDebug dbg, const AtomEntry& e);

class AtomFeed
{
public:
	static QFuture<AtomFeed*> parse(QIODevice* device);
	static QFuture<AtomFeed*> parse(const QString& fileName);
	
	~AtomFeed();
	
	bool hasError() const { return m_error; }
	
	QString title() const { return m_title; }
	QList<AtomEntry> entries() const { return m_entries; }

private:
	AtomFeed();
	
	AtomFeed* doParseStream(QIODevice* device);
	AtomFeed* doParseFilename(const QString& fileName);
	
	void parseFeed(QXmlStreamReader& s);
	
	bool m_error;
	QString m_title;
	QList<AtomEntry> m_entries;
};


#endif
