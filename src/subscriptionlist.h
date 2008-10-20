#ifndef SUBSCRIPTIONLIST_H
#define SUBSCRIPTIONLIST_H

#include <QDebug>
#include <QList>
#include <QPair>
#include <QXmlStreamReader>

class QSqlQuery;

typedef QPair<QString, QString> Category;

// Contains the information about a subscription to a feed
// Includes the folders/tags/categories.
// Does not contain atom entries.
class Subscription {
public:
	Subscription(QXmlStreamReader& s);
	Subscription(const QSqlQuery& query, Database* db);
	Subscription(const QString& id, const QString& title);
	
	const QString& id() const { return id_; }
	const QString& title() const { return title_; }
	const QString& sortid() const { return sortid_; }
	const QString& xpath() const { return xpath_; }
	void setXpath(const QString& xpath);
	const QList<Category>& categories() const { return categories_; }
	void addCategory(const Category& category);
	void removeCategory(const QString& category);

private:
	void parseCategories(QXmlStreamReader& s);
	void parseCategory(QXmlStreamReader& s);
	void categoriesLoaded(const QSqlQuery& query);

private:
	QString id_;
	QString title_;
	QString sortid_;
	QString xpath_;
	// Pair: id, name
	QList<Category> categories_;
};

class SubscriptionList {
public:
	SubscriptionList(const SubscriptionList& other);
	SubscriptionList(QXmlStreamReader& s);
	const QList<Subscription*>& subscriptions() const { return subscriptions_; }  
private:
	QList<Subscription*> subscriptions_;

};

QDebug operator <<(QDebug dbg, const Subscription& s);

#endif
