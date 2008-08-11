#ifndef SUBSCRIPTIONLIST_H
#define SUBSCRIPTIONLIST_H

#include <QDebug>
#include <QList>
#include <QPair>
#include <QXmlStreamReader>

class Subscription {
public:
	Subscription(QXmlStreamReader& s);
	QString id() const { return id_; }
	QString title() const { return title_; }
	QString sortid() const { return sortid_; }
	const QList<QPair<QString, QString> >& categories() const { return categories_; }

private:
	void parseCategories(QXmlStreamReader& s);
	void parseCategory(QXmlStreamReader& s);

	QString id_;
	QString title_;
	QString sortid_;
	// Pair: id, name
	QList<QPair<QString, QString> > categories_;
};

class SubscriptionList : public QObject {
	Q_OBJECT
public:
	SubscriptionList(QXmlStreamReader& s);
	const QList<Subscription>& subscriptions() const { return subscriptions_; }  
private:
	QList<Subscription> subscriptions_;

};

QDebug operator <<(QDebug dbg, const Subscription& s);

#endif
