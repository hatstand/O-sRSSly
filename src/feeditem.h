#ifndef FEEDITEM_H
#define FEEDITEM_H

#include "atomfeed.h"
#include "subscriptionlist.h"
#include "treeitem.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using boost::shared_ptr;
using boost::weak_ptr;

class ReaderApi;

class FeedItemData : public QObject {
	Q_OBJECT
public:
	FeedItemData(const Subscription& s, ReaderApi* api);
	FeedItemData(const QSqlQuery& query, ReaderApi* api);
	~FeedItemData();
	void setRead(const AtomEntry& e);

	const AtomFeed::AtomList& entries() { return feed_.entries(); }
	const Subscription& subscription() { return subscription_; }

	void update();
	int update(const AtomEntry& e);

	void addCategory(const QPair<QString,QString>& category);
	void removeCategory(const QString& category);

	void setStarred(const AtomEntry& e, bool starred = true);

	void setXpath(const QString& xpath);
	const QString& xpath() const;

	void save();

private slots:
	int update(const AtomFeed& feed);

signals:
	void rowsInserted(int from, int to);

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
	QVariant data(int column, int role = Qt::DisplayRole) const;
	QIcon icon() const { return sIcon; }
	const Subscription& subscription() const { return data_->subscription(); }
	const AtomEntry& entry(const QModelIndex& index) const;

	QString real_id(const QModelIndex& index) const;

	virtual QString summary(const QModelIndex& index) const;
	QString content(const QModelIndex& index) const;
	// Sets an atom entry to read status (local only).
	void addCategory(const QPair<QString,QString>& category);
	void setStarred(const QModelIndex& index, bool starred = true);

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
	virtual int rowCount(const QModelIndex& parent) const;

	virtual bool canFetchMore(const QModelIndex& parent) const { return true; }
	virtual void fetchMore(const QModelIndex& parent);
	
	const weak_ptr<FeedItemData> feedItemData() const { return data_; }

private slots:
	void feedRowsInserted(int from, int to);

private:
	shared_ptr<FeedItemData> data_;
	
	static QIcon sIcon;
};

#endif
