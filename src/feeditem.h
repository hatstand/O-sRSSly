#ifndef FEEDITEM_H
#define FEEDITEM_H

#include "atomfeed.h"
#include "subscriptionlist.h"
#include "treeitem.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

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
	struct Data {
		Data(const Subscription& s) : subscription_(s) {}
		void update(const AtomFeed& feed);
		// The feed id & title etc.
		Subscription subscription_;
		// The atom entries.
		AtomFeed feed_;
	};

	FeedItem(TreeItem* parent, Data* data);
	int columnCount() const;
	QVariant data(int column) const;
	TreeItem::Type rtti() const { return TreeItem::Feed; }
	const Subscription& subscription() const { return data_->subscription_; }
	AtomFeed* entries() { return &data_->feed_; }
	const AtomEntry& entry(const QModelIndex& index) const;

	virtual QString summary(const QModelIndex& index) const;
	// Sets an atom entry to read status (local only).
	void setRead(const QModelIndex& index);

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

private:
	shared_ptr<Data> data_;
};

#endif
