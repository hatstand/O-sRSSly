#include "atomfeed.h"
#include "xmlutils.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>

using namespace XmlUtils;

std::size_t hash_value(const QString& s)
{
	return qHash(s);
}

QDebug operator <<(QDebug dbg, const AtomEntry& e)
{
	dbg.nospace() << "AtomEntry(" << e.title << ", " << e.id << ", " << (e.read ? "read" : "unread") << ")";
	return dbg.space();
}

AtomFeed::AtomFeed() {
	init();
}

AtomFeed::AtomFeed(const QUrl& url, QIODevice* device)
	: m_url(url)
{
	init();
	parse(device);
}

AtomFeed::~AtomFeed()
{
}

void AtomFeed::init()
{
	m_error = false;
	m_dbId = -1;
}

void AtomFeed::parse(QIODevice* device)
{
	QXmlStreamReader s(device);
	
	while (!s.atEnd())
	{
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type)
		{
		case QXmlStreamReader::StartElement:
			if (s.name() == "feed")
				parseFeed(s);
			else
				ignoreElement(s);
			
			break;
		
		default:
			break;
		}
	}
	
	if (s.hasError())
		m_error = true;
}

void AtomFeed::parseFeed(QXmlStreamReader& s)
{
	while (!s.atEnd())
	{
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type)
		{
		case QXmlStreamReader::StartElement:
			if (s.name() == "title")
				m_title = s.readElementText();
			else if (s.name() == "id")
				m_id = s.readElementText();
			else if (s.name() == "entry")
				m_entries.insert(AtomEntry(s));
			else
				ignoreElement(s);
			
			break;
			
		case QXmlStreamReader::EndElement:
			if (s.name() == "feed")
				return;
			break;
		
		default:
			break;
		}
	}
}

void AtomFeed::merge(const AtomFeed& other) {
	if (!m_id.isEmpty() && m_id != other.m_id) {
		qWarning() << "Attempting to merge non-matching feeds";
		qDebug() << m_id << other.m_id;
		return;
	}

	foreach (const AtomEntry& e, other.m_entries) {
		m_entries.insert(e);
	}
}

void AtomFeed::setRead(const AtomEntry& e) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		AtomEntry f(*it);
		f.read = true;

		m_entries.get<hash>().replace(it, f);
	}
}

AtomEntry::AtomEntry(QXmlStreamReader& s)
	: read(false)
{
	while (!s.atEnd())
	{
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type)
		{
		case QXmlStreamReader::StartElement:
			if (s.name() == "title") {
				QString temp = s.readElementText();
				XmlUtils::unescape(temp);
				title = temp;
			} else if (s.name() == "id")
				id = s.readElementText();
			else if (s.name() == "summary")
				summary = s.readElementText();
			else if (s.name() == "category" && s.attributes().value("label") == "read")
				read = true;
			else if (s.name() == "updated")
				date = QDateTime::fromString(s.readElementText(), Qt::ISODate);
			else
				ignoreElement(s);
			
			break;
			
		case QXmlStreamReader::EndElement:
			if (s.name() == "entry")
				return;
			break;
		
		default:
			break;
		}
	}
}

void AtomFeed::load(const QString& id)
{
	QSqlQuery query;
	query.prepare("SELECT ROWID, url, id, title FROM feeds WHERE id=:id");
	query.bindValue(":id", id);
	query.exec();
	
	load(query);
}

void AtomFeed::load(QSqlQuery& query)
{
	m_dbId = query.value(0).toLongLong();
	m_url = query.value(1).toString();
	m_id = query.value(2).toString();
	m_title = query.value(3).toString();
}

void AtomFeed::save() const
{
	QSqlQuery query;
	if (m_dbId == -1)
	{
		query.prepare("INSERT INTO feeds (url, id, title) VALUES (:url, :id, :title)");
		
		query.bindValue(":url", m_url.toString());
		query.bindValue(":id", m_id);
		query.bindValue(":title", m_title);
		
		query.exec();
		
		const_cast<AtomFeed*>(this)->m_dbId = query.lastInsertId().toLongLong();
	}
	else
	{
		query.prepare("UPDATE FEEDS SET title=:title");
		
		query.bindValue(":title", m_title);
		
		query.exec();
	}
}

QList<AtomFeed*> AtomFeed::loadAll()
{
	QList<AtomFeed*> ret;
	
	QSqlQuery query("SELECT ROWID, url, id, title FROM feeds");
	while (query.next())
	{
	}

	return ret;
}

QDebug operator <<(QDebug dbg, const AtomFeed& f)
{
	dbg.nospace() << "AtomFeed(" << f.title() << ", " << f.id() << ")\n";
	for(AtomFeed::AtomList::const_iterator it = f.entries().begin(); it != f.entries().end(); ++it)
	{
		dbg.nospace() << "  " << *it;
		dbg.nospace() << "\n";
	}
	return dbg.space();
}

