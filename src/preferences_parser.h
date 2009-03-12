#ifndef PREFERENCES_PARSER_H
#define PREFERENCES_PARSER_H

#include <boost/utility.hpp>

#include <QMap>
#include <QString>
#include <QXmlStreamReader>

class PreferencesParser : boost::noncopyable {
public:
	PreferencesParser(QXmlStreamReader& s);
	const QMap<QString, QString>& getPreferences() const { return prefs_; }

private:
	void parseObject(QXmlStreamReader& s);

	bool inside_list_;
	QMap<QString, QString> prefs_;
};

#endif
