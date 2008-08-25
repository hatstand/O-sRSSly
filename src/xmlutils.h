#ifndef XMLUTILS_H
#define XMLUTILS_H

#include <QRegExp>
#include <QXmlStreamReader>

namespace XmlUtils {
	void ignoreElement(QXmlStreamReader& s);
	QString& unescape(QString& s);
	QString& escape(QString& s);
	QString& stripTags(QString& s);
	
	QString unescaped(const QString& s);
	QString escaped(const QString& s);
};

#endif
