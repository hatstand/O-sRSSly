#include "atomfeed.h"
#include "xmlutils.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QtDebug>

using namespace XmlUtils;

QDebug operator <<(QDebug dbg, const AtomEntry& e)
{
	dbg.nospace() << "AtomEntry(" << e.title << ", " << e.id << ", " << (e.read ? "read" : "unread") << ")";
	return dbg.space();
}

AtomFeed::AtomFeed() {
	init();
}

AtomFeed::AtomFeed(const QString& fileName)
{
	init();
	
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	parse(&file);
}

AtomFeed::AtomFeed(QIODevice* device)
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
				m_entries << AtomEntry(s);
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

AtomEntry::AtomEntry(QXmlStreamReader& s)
	: read(false)
{
	while (!s.atEnd())
	{
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type)
		{
		case QXmlStreamReader::StartElement:
			if (s.name() == "title")
				title = s.readElementText();
			else if (s.name() == "id")
				id = s.readElementText();
			else if (s.name() == "summary")
				summary = s.readElementText();
			else if (s.name() == "category" && s.attributes().value("label") == "read")
				read = true;
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

QDebug operator <<(QDebug dbg, const AtomFeed& f)
{
	dbg.nospace() << "AtomFeed(" << f.title() << ", " << f.id() << ")\n";
	foreach (const AtomEntry& e, f.entries())
	{
		dbg.nospace() << "  " << e;
		dbg.nospace() << "\n";
	}
	return dbg.space();
}

