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
