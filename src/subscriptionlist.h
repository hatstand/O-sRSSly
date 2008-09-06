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
	Subscription(const QSqlQuery& query);
	Subscription(const QString& id, const QString& title);
	
	QString id() const { return id_; }
	QString title() const { return title_; }
	QString sortid() const { return sortid_; }
	const QList<Category>& categories() const { return categories_; }
	void addCategory(const Category& category);
	void removeCategory(const QString& category);

private:
	void parseCategories(QXmlStreamReader& s);
	void parseCategory(QXmlStreamReader& s);

	QString id_;
	QString title_;
	QString sortid_;
	// Pair: id, name
	QList<Category> categories_;
};

class SubscriptionList : public QObject {
	Q_OBJECT
public:
	SubscriptionList(const SubscriptionList& other);
	SubscriptionList(QXmlStreamReader& s);
	const QList<Subscription>& subscriptions() const { return subscriptions_; }  
private:
	QList<Subscription> subscriptions_;

};

QDebug operator <<(QDebug dbg, const Subscription& s);

#endif
