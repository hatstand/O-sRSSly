#include "xmlutils.h"

void XmlUtils::ignoreElement(QXmlStreamReader& s)
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

QString& XmlUtils::unescape(QString& s) {
	s.replace("&quot;", "\"");
	s.replace("&amp;", "&");
	s.replace("&lt;", "<");
	s.replace("&gt;", ">");
	s.replace("&#39;", "'");

	return s;
}
