#include "atomfeed.h"
#include "xmlutils.h"
#include "database.h"

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

AtomFeed::AtomFeed(const QString& id)
	: m_error(false),
	  m_id(id)
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
	  m_id(query.value(0).toString())
{
	QSqlQuery entryQuery;
	entryQuery.prepare("SELECT title, id, summary, content, date, link, read, starred, author, shared_by FROM Entry WHERE feedId=:feedId");
	entryQuery.bindValue(":feedId", m_id);
	if (!entryQuery.exec())
		Database::handleError(entryQuery.lastError());
	
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

void AtomFeed::add(const AtomEntry& e) {
	if (!m_id.isEmpty() && m_id != e.source) {
		qWarning() << "Attempting to add non-matching entry";
		qDebug() << m_id << e.source;
		return;
	}

	m_entries.insert(e);
}

void AtomFeed::setRead(const AtomEntry& e) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		AtomEntry f(*it);
		f.read = true;

		m_entries.get<hash>().replace(it, f);

		// Sync change to sql.
		f.update();
	}
}

void AtomFeed::setStarred(const AtomEntry& e, bool starred) {
	AtomEntries::iterator it = m_entries.get<hash>().find(e.id);

	if (it != m_entries.get<hash>().end()) {
		AtomEntry f(*it);
		f.starred = starred;

		m_entries.get<hash>().replace(it, f);

		// Sync change to sql.
		f.update();
	}
}

void AtomFeed::saveEntries() {
	QSqlQuery query;
	query.prepare("REPLACE INTO Entry (feedId, title, id, summary, content, date, link, read, starred, author, shared_by) VALUES (:feedId, :title, :id, :summary, :content, :date, :link, :read, :starred, :author, :shared_by)");
	query.bindValue(":feedId", m_id);
	
	for (AtomList::const_iterator it = entries().begin(); it != entries().end(); ++it) {
		const AtomEntry& entry(*it);
		
		query.bindValue(":title", entry.title);
		query.bindValue(":id", entry.id);
		query.bindValue(":summary", entry.summary);
		query.bindValue(":content", entry.content);
		query.bindValue(":date", entry.date.toString());
		query.bindValue(":link", entry.link.toString());
		query.bindValue(":read", QVariant(entry.read).toString());
		query.bindValue(":starred", QVariant(entry.starred).toString());
		query.bindValue(":author", entry.author);
		query.bindValue(":shared_by", entry.shared_by);
		if (!query.exec())
			Database::handleError(query.lastError());
	}
}

AtomEntry::AtomEntry(QXmlStreamReader& s)
	: read(false),
	  starred(false)
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

void AtomEntry::update() const {
	QSqlQuery query;
	query.prepare("UPDATE Entry SET read=:read, starred=:starred WHERE id=:id");
	query.bindValue(":id", id);
	query.bindValue(":read", QVariant(read).toString());
	query.bindValue(":starred", QVariant(starred).toString());
	if (!query.exec())
		Database::handleError(query.lastError());
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

