#ifndef FEEDITEM_H
#define FEEDITEM_H

#include "atomfeed.h"
#include "subscriptionlist.h"
#include "treeitem.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

class FeedItem : public TreeItem {
	Q_OBJECT
public:
	struct Data {
		Data(const Subscription& s) : subscription_(s) {}
		void update(const AtomFeed& feed);
		Subscription subscription_;
		AtomFeed feed_;
	};

	FeedItem(TreeItem* parent, Data* data);
	int columnCount() const;
	QVariant data(int column) const;
	TreeItem::Type rtti() const { return TreeItem::Feed; }
	const Subscription& subscription() const { return data_->subscription_; }
	AtomFeed* entries() { return &data_->feed_; }

	virtual QString summary(const QModelIndex& index) const;

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

private:
	shared_ptr<Data> data_;
};

#endif
