#include "atomfeed.h"
#include "xmlutils.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>

const char* AtomFeed::kReaderXmlNamespace = "http://www.google.com/schemas/reader/atom/";

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

AtomFeed::AtomFeed()
	: m_error(false)
{
}

AtomFeed::AtomFeed(const QUrl& url, QIODevice* device)
	: m_error(false),
	  m_url(url)
{
	parse(device);
}

AtomFeed::AtomFeed(const QSqlQuery& query)
	: m_error(false),
	  m_id(query.value(1).toString())
{
	QSqlQuery entryQuery;
	entryQuery.prepare("SELECT ROWID, title, id, summary, content, date, link, read, starred FROM Entry WHERE feedId=:feedId");
	entryQuery.bindValue(":feedId", query.value(0).toLongLong());
	entryQuery.exec();
	
	while (entryQuery.next())
		m_entries.insert(AtomEntry(entryQuery));
}

AtomFeed::~AtomFeed()
{
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
			else if (s.name() == "id") {
				m_id = s.readElementText();
				m_id.remove(QRegExp("^tag:google.com,2005:reader/"));
			} else if (s.name() == "entry")
				m_entries.insert(AtomEntry(s));
			else if (s.namespaceUri() == kReaderXmlNamespace && s.name() == "continuation")
				m_continuation = s.readElementText();
			else
				ignoreElement(s);
			
			break;
			
		case QXmlStreamReader::EndElement:
			if (s.name() == "feed") {
				return;
			}
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

	m_continuation = other.m_continuation;
}

void AtomFeed::setRead(const AtomEntry& e) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		AtomEntry f(*it);
		f.read = true;

		m_entries.get<hash>().replace(it, f);
	}
}

void AtomFeed::saveEntries(qint64 feedId) {
	QSqlQuery query;
	query.prepare("INSERT INTO Entry (feedId, title, id, summary, content, date, link, read) VALUES (:feedId, :title, :id, :summary, :content, :date, :link, :read, :starred)");
	query.bindValue(":feedId", feedId);
	
	for (AtomList::const_iterator it = entries().begin(); it != entries().end(); ++it) {
		const AtomEntry& entry(*it);
		
		if (entry.rowid != -1)
			continue;
		
		query.bindValue(":title", entry.title);
		query.bindValue(":id", entry.id);
		query.bindValue(":summary", entry.summary);
		query.bindValue(":content", entry.content);
		query.bindValue(":date", entry.date.toString());
		query.bindValue(":link", entry.link.toString());
		query.bindValue(":read", QVariant(entry.read).toString());
		query.bindValue(":starred", QVariant(entry.starred).toString());
		query.exec();
	}
}

AtomEntry::AtomEntry(QXmlStreamReader& s)
	: read(false),
	  starred(false),
	  rowid(-1)
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
			else if (s.name() == "category" && s.attributes().value("label") == "starred")
				starred = true;
			else if (s.name() == "updated")
				date = QDateTime::fromString(s.readElementText(), Qt::ISODate);
			else if (s.name() == "link")
				link = QUrl(s.attributes().value("href").toString());
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

AtomEntry::AtomEntry(const QSqlQuery& query) {
	rowid = query.value(0).toLongLong();
	title = query.value(1).toString();
	id = query.value(2).toString();
	summary = query.value(3).toString();
	content = query.value(4).toString();
	date = QDateTime::fromString(query.value(5).toString());
	link = query.value(6).toString();
	read = query.value(7).toBool();
	starred = query.value(8).toBool();
}

const QString& AtomEntry::previewText() const {
	using namespace XmlUtils;
	
	if (previewText_.isNull())
	{
		// Mwhahaha
		QString& previewTextRef(const_cast<AtomEntry*>(this)->previewText_);
		previewTextRef = summary.simplified();
		unescape(stripTags(previewTextRef));
	}
	return previewText_;
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

