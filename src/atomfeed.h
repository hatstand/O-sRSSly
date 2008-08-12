#ifndef ATOMFEED_H
#define ATOMFEED_H

#include <QString>
#include <QList>
#include <QXmlStreamReader> // Do not change to class QXmlStreamReader (gcc 4.0.1)

class QIODevice;

class AtomEntry
{
public:
	AtomEntry(QXmlStreamReader& s);
	
	QString title;
	QString id;
	QString summary;
	QString content;
	bool read;
};

QDebug operator <<(QDebug dbg, const AtomEntry& e);

class AtomFeed
{
public:
	AtomFeed(const QString& fileName);
	AtomFeed(QIODevice* device);
	~AtomFeed();
	
	bool hasError() const { return m_error; }
	
	QString id() const { return m_id; }
	QString title() const { return m_title; }
	QList<AtomEntry> entries() const { return m_entries; }

private:
	void init();
	void parse(QIODevice* device);
	void parseFeed(QXmlStreamReader& s);
	
	bool m_error;
	
	QString m_id;
	QString m_title;
	QList<AtomEntry> m_entries;
};

QDebug operator <<(QDebug dbg, const AtomFeed& f);


#endif
