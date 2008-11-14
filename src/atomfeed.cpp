#include "atomfeed.h"
#include "xmlutils.h"
#include "database.h"

#include <QFile>
#include <QList>
#include <QtDebug>
#include <QVariant>
#include <QXmlStreamReader>

#include <boost/bind.hpp>
using boost::bind;

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

AtomFeed::AtomFeed(const QString& id, Database* db)
	: m_error(false),
	  m_id(id),
	  db_(db)
{
}

AtomFeed::AtomFeed(const QUrl& url, QIODevice* device, Database* db)
	: m_error(false),
	  m_url(url),
	  db_(db)
{
	parse(device);
}

AtomFeed::AtomFeed(const QSqlQuery& query, Database* db)
	: m_error(false),
	  m_id(query.value(0).toString()),
	  db_(db)
{
	db_->pushQuery(
		"SELECT title, id, summary, content, date, link, read, starred, author, shared_by, shared "
		"FROM Entry WHERE feedId=?",
		QList<QVariant>() << m_id,
		bind(&AtomFeed::addEntries, this, _1));
}

void AtomFeed::addEntries(const QSqlQuery& query) {
	QSqlQuery mutable_query(query);
	if (query.boundValue(":feedId") == m_id) {
		while (mutable_query.next())
			m_entries.insert(AtomEntry(mutable_query));
	}
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
			else if (s.name() == "updated")
				m_updated = QDateTime::fromString(s.readElementText(), Qt::ISODate);
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

void AtomFeed::merge(const AtomFeed& other, bool definitive) {
	if (!m_id.isEmpty() && m_id != other.m_id) {
		qWarning() << "Attempting to merge non-matching feeds";
		qDebug() << m_id << other.m_id;
		return;
	}

	if (definitive) {
		foreach (const AtomEntry& e, other.m_entries) {
			AtomEntries::iterator it = m_entries.get<hash>().find(e.id);
			if (it == m_entries.get<hash>().end()) {
				m_entries.insert(e);
			} else if (e.date > it->date) {
				m_entries.modify(it, update_entry(e));
			}
		}
	} else {
		foreach (const AtomEntry& e, other.m_entries) {
			m_entries.insert(e);
		}
	}

	m_continuation = other.m_continuation;
	m_updated = QDateTime::currentDateTime();
}

void AtomFeed::add(const AtomEntry& e, bool definitive) {
	if (!m_id.isEmpty() && m_id != e.source) {
		qWarning() << "Attempting to add non-matching entry";
		qDebug() << m_id << e.source;
		return;
	}

	if (definitive) {
		AtomEntries::iterator it = m_entries.get<hash>().find(e.id);
		if (it == m_entries.get<hash>().end()) {
			m_entries.insert(e);
		} else if (e.date > it->date) {
			m_entries.modify(it, update_entry(e));
		}
	} else {
		m_entries.insert(e);
	}
}

void AtomFeed::setRead(const AtomEntry& e) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		m_entries.modify(it, set_read());

		// Sync change to sql.
		it->update(db_);
	}
}

void AtomFeed::setStarred(const AtomEntry& e, bool starred) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		m_entries.modify(it, set_starred(starred));

		// Sync change to sql.
		it->update(db_);
	}
}

void AtomFeed::saveEntries() {
	QString query("REPLACE INTO Entry (feedId, title, id, summary, content, date, link, read, starred, author, shared_by, shared) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	
	for (AtomList::const_iterator it = entries().begin(); it != entries().end(); ++it) {
		const AtomEntry& entry(*it);
		QList<QVariant> bind_values;
		bind_values << m_id;
		
		bind_values << entry.title;
		bind_values << entry.id;
		bind_values << entry.summary;
		bind_values << entry.content;
		bind_values << entry.date.toString();
		bind_values << entry.link.toString();
		bind_values << QVariant(entry.read);
		bind_values << QVariant(entry.starred);
		bind_values << entry.author;
		bind_values << entry.shared_by;
		bind_values << entry.shared;

		db_->pushQuery(query, bind_values);
	}
}

AtomEntry::AtomEntry(QXmlStreamReader& s)
	: read(false),
	  starred(false),
	  shared(false)
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
			else if (s.name() == "category" && s.attributes().value("label") == "broadcast")
				shared = true;
			else if (s.name() == "updated")
				date = QDateTime::fromString(s.readElementText(), Qt::ISODate);
			else if (s.name() == "link" && s.attributes().value("rel") == "alternate")
				link = QUrl(s.attributes().value("href").toString());
			else if (s.name() == "link" && s.attributes().value("rel") == "via") {
				QString shared_link = s.attributes().value("href").toString();
				QRegExp exp("user/[0-9]+/state/com.google/broadcast");
				if (exp.indexIn(shared_link) > -1) {
					source = exp.cap();
					shared_by = s.attributes().value("title").toString();
				}
			}
			else if (s.name() == "source" && source.isEmpty())
				parseSource(s);
			else if (s.name() == "content")
				content = s.readElementText();
			else if (s.name() == "author" && s.attributes().value(
				AtomFeed::kReaderXmlNamespace, "unknown-author") != "true") {
				author = parseAuthor(s);
			}
			else {
				ignoreElement(s);
			}
			
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
	title = query.value(0).toString();
	id = query.value(1).toString();
	summary = query.value(2).toString();
	content = query.value(3).toString();
	date = QDateTime::fromString(query.value(4).toString());
	link = query.value(5).toString();
	read = query.value(6).toBool();
	starred = query.value(7).toBool();
	author = query.value(8).toString();
	shared_by = query.value(9).toString();
	shared = query.value(10).toBool();
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

void AtomEntry::parseSource(QXmlStreamReader& s) {
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "id") {
					source = s.readElementText();
					source.remove(QRegExp("^tag:google.com,2005:reader/"));
				}
				else
					ignoreElement(s);

				break;
			
			case QXmlStreamReader::EndElement:
				if (s.name() == "source")
					return;
				break;

			default:
				break;
		}
	}
}

QString AtomEntry::parseAuthor(QXmlStreamReader& s) {
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "name") {
					return s.readElementText();
				}
				else
					ignoreElement(s);

				break;
			
			case QXmlStreamReader::EndElement:
				if (s.name() == "author")
					return QString::null;
				break;

			default:
				break;
		}
	}

	return QString::null;
}

void AtomEntry::update(Database* db) const {
	QString query = "UPDATE Entry SET read=?, starred=? WHERE id=?";
	QList<QVariant> args;
	args << id;
	args << QVariant(read);
	args << QVariant(starred);
	db->pushQuery(query, args);
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
