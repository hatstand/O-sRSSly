#include "subscriptionlist.h"
#include "xmlutils.h"

using namespace XmlUtils;

Subscription::Subscription(QXmlStreamReader& s) {
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "string") {
					QString name = s.attributes().value("name").toString();
					if (!name.isEmpty()) {
						if (name == "id")
							id_ = s.readElementText();
						else if (name == "title")
							title_ = s.readElementText();
						else if (name == "sortid")
							sortid_ = s.readElementText();
					}
				} else if (s.name() == "list" && s.attributes().value("name") == "categories") {
					parseCategories(s);
				} else {
					ignoreElement(s);
				}
				
				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "object")
					return;

				break;

			default:
				break;
		}
	}
}

void Subscription::parseCategories(QXmlStreamReader& s) {
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "object")
					parseCategory(s);
				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "list")
					return;

				break;

			default:
				break;
		}
	}
}

void Subscription::parseCategory(QXmlStreamReader& s) {
	QString id, name;

	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "string") {
					if (s.attributes().value("name") == "id")
						id = s.readElementText();
					else if (s.attributes().value("name") == "label")
						name = s.readElementText();
				} else
					ignoreElement(s);
				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "object") {
					if (!id.isEmpty() && !name.isEmpty())
						categories_ << qMakePair(id, name);

					return;
				}

				break;

			default:
				break;
		}
	}
}

void Subscription::addCategory(const QPair<QString,QString>& category) {
	categories_ << category;
}

SubscriptionList::SubscriptionList(const SubscriptionList& other) {
	subscriptions_ = other.subscriptions_;
}

SubscriptionList::SubscriptionList(QXmlStreamReader& s) {
	bool inside_list = false;
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "list")
					inside_list = true;
				else if (inside_list && s.name() == "object")
					subscriptions_ << Subscription(s);
				else if (s.name() != "object")
					ignoreElement(s);

				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "list")
					return;

				break;

			default:
				break;
		}
	}
}

QDebug operator <<(QDebug dbg, const Subscription& s) {
	dbg.nospace() << "Subscription(" << s.id() << ", " << s.title() << ", " << s.sortid() << ", Categories(";
	
	typedef QPair<QString, QString> Category;
	foreach (const Category& c, s.categories()) {
		dbg.nospace() << "[" << c.first << ", " << c.second << "],";
	}

	dbg.nospace() << "))";

	return dbg.space();
}
