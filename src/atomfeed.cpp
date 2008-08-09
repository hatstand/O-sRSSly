#include "atomfeed.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QtDebug>

void ignoreElement(QXmlStreamReader& s)
{
	int level = 1;
	while (level != 0 && !s.atEnd())
	{
		QXmlStreamReader::TokenType type = s.readNext();
		if (type == QXmlStreamReader::StartElement)
			level++;
		else if (type == QXmlStreamReader::EndElement)
			level--;
	}
}

QDebug operator <<(QDebug dbg, const AtomEntry& e)
{
	dbg.nospace() << "AtomEntry(" << e.title << ", " << e.id << ")";
	return dbg.space();
}

AtomFeed::AtomFeed()
	: m_error(false)
{
}

AtomFeed::~AtomFeed()
{
}

void AtomFeed::parse(const QString& fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	return parse(&file);
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

