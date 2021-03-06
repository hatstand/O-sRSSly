#include "database.h"
#include "settings.h"
#include "subscriptionlist.h"
#include "xmlutils.h"

#include <QSqlQuery>
#include <QVariant>

#include <boost/bind.hpp>
using boost::bind;

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

Subscription::Subscription(const QSqlQuery& query, Database* db) {
	Q_UNUSED(db);
	// Load data about the subscription itself
	id_ = query.value(0).toString();
	title_ = query.value(1).toString();
	sortid_ = query.value(2).toString();
	
	// Now load the list of categories
	// Now done by FeedsModel
	/*db->pushQuery(
		"SELECT Tag.id, Tag.title FROM Tag "
		"INNER JOIN FeedTagMap ON Tag.id=FeedTagMap.tagId "
		"WHERE FeedTagMap.feedId=:feedId",
		QList<QVariant>() << id_,
		bind(&Subscription::categoriesLoaded, this, _1));*/
}

void Subscription::categoriesLoaded(const QSqlQuery& query) {
	qDebug() << __PRETTY_FUNCTION__;

	QSqlQuery mutable_query(query);
	while (mutable_query.next())
		addCategory(Category(mutable_query.value(0).toString(), mutable_query.value(1).toString()));
}

Subscription::Subscription(const QString& id, const QString& title)
	: id_(id),
	  title_(title) {
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

void Subscription::addCategory(const Category& category) {
	categories_ << category;
}

void Subscription::removeCategory(const QString& category) {
	QList<Category>::iterator it = categories_.begin();
	for (; it != categories_.end(); ++it) {
		if (it->first == category) {
			categories_.erase(it);
			break;
		}
	}
}

void Subscription::setXpath(const QString& xpath) {
	qDebug() << __PRETTY_FUNCTION__ << xpath;
	xpath_ = xpath;
	if (!xpath_.isEmpty())
		Settings::instance()->setBehaviour(id_, Settings::Behaviour_Webclip);
	else
		Settings::instance()->setBehaviour(id_, Settings::Behaviour_Auto);
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
					subscriptions_ << new Subscription(s);
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
	
	foreach (const Category& c, s.categories()) {
		dbg.nospace() << "[" << c.first << ", " << c.second << "],";
	}

	dbg.nospace() << "))";

	return dbg.space();
}
