#ifndef XMLUTILS_H
#define XMLUTILS_H

#include <QRegExp>
#include <QXmlStreamReader>

namespace XmlUtils {
	void ignoreElement(QXmlStreamReader& s);
	QString& unescape(QString& s);
	QString& stripTags(QString& s);
};

#endif
