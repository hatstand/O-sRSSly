#ifndef FEEDITEM_H
#define FEEDITEM_H

#include "atomfeed.h"
#include "subscriptionlist.h"
#include "treeitem.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

class ReaderApi;

class FeedItemData : public QObject {
	Q_OBJECT
public:
	FeedItemData(const Subscription& s, ReaderApi* api) : subscription_(s), api_(api) {}
	~FeedItemData();
	void setRead(const AtomEntry& e);

	const AtomFeed::AtomList& entries() { return feed_.entries(); }
	const Subscription& subscription() { return subscription_; }

	void update();

	void addCategory(const QPair<QString,QString>& category);
	void removeCategory(const QString& category);

private slots:
	void update(const AtomFeed& feed);

signals:
	void updated();

private:
	// The feed id & title etc.
	Subscription subscription_;
	// The atom entries.
	AtomFeed feed_;
	ReaderApi* api_;
};

/*
 * Represents an actual atom feed/subscription.
 */
class FeedItem : public TreeItem {
	Q_OBJECT
public:
	/*
	 * Shared data
	 * A FeedItem may appear multiple times in the same tree.
	 * The data only exists once (shared_ptr).
	 */

	FeedItem(TreeItem* parent, shared_ptr<FeedItemData> data);
	int columnCount() const;
	QVariant data(int column) const;
	TreeItem::Type rtti() const { return TreeItem::Feed; }
	const Subscription& subscription() const { return data_->subscription(); }
	const AtomEntry& entry(const QModelIndex& index) const;

	virtual QString summary(const QModelIndex& index) const;
	// Sets an atom entry to read status (local only).
	void setRead(const QModelIndex& index);
	void addCategory(const QPair<QString,QString>& category);

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

	virtual bool canFetchMore(const QModelIndex& parent) const { return true; }
	virtual void fetchMore(const QModelIndex& parent);

private slots:
	void feedUpdated();

private:
	shared_ptr<FeedItemData> data_;
};

#endif
