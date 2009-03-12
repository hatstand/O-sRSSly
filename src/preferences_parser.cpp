#include "preferences_parser.h"

#include "xmlutils.h"

using XmlUtils::ignoreElement;

PreferencesParser::PreferencesParser(QXmlStreamReader& s)
	: inside_list_(false) {
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "list") {
					inside_list_ = true;
				} else if (inside_list_ && s.name() == "object") {
					parseObject(s);
				} else if (s.name() != "object") {
					ignoreElement(s);
				}
				break;
			case QXmlStreamReader::EndElement:
				break;
			default:
				break;
		}
	}
}

void PreferencesParser::parseObject(QXmlStreamReader& s) {
	QString current_id, current_value;
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "string") {
					QStringRef name = s.attributes().value("name");
					if (name == "id") {
						current_id = s.readElementText();
					} else if (name == "value") {
						current_value = s.readElementText();
					}
				} else {
					ignoreElement(s);
				}
				break;
			case QXmlStreamReader::EndElement:
				if (s.name() == "object") {
					if (!current_id.isEmpty() && !current_value.isEmpty()) {
						prefs_.insert(current_id, current_value);
					}
					return;
				}
				break;
			default:
				break;
		}
	}
}
