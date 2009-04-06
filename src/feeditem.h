#ifndef FEEDITEM_H
#define FEEDITEM_H

#include "atomfeed.h"
#include "subscriptionlist.h"
#include "treeitem.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Database;
class ReaderApi;

class FeedItemData : public QObject {
	Q_OBJECT
public:
	FeedItemData(Subscription* s, ReaderApi* api, Database* db);
	FeedItemData(const QSqlQuery& query, ReaderApi* api, Database* db);
	virtual ~FeedItemData();
	void setRead(const AtomEntry& e);

	const AtomFeed::AtomList& entries() { return feed_.entries(); }
	const Subscription& subscription() { return *subscription_; }

	void update();
	int update(const AtomEntry& e, bool definitive = false);

	void addCategory(const QPair<QString,QString>& category, bool update_account = true);
	void removeCategory(const QString& category);

	void setStarred(const AtomEntry& e, bool starred = true);

	void setXpath(const QString& xpath);
	void setShared(const AtomEntry& e);
	const QString& xpath() const;

	Database* database() { return db_; }

	void save();

	int unread() const;

public slots:
	int update(const AtomFeed& feed);

signals:
	void rowsInserted(int from, int to);

private:
	// The feed id & title etc.
	boost::scoped_ptr<Subscription> subscription_;
	// The atom entries.
	AtomFeed feed_;
	ReaderApi* api_;
	Database* db_;
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

	FeedItem(TreeItem* parent, boost::shared_ptr<FeedItemData> data);
	virtual ~FeedItem() {};
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

	virtual bool canFetchMore(const QModelIndex&) const { return true; }
	virtual void fetchMore(const QModelIndex& parent);
	
	const boost::weak_ptr<FeedItemData> feedItemData() const { return data_; }

private slots:
	void feedRowsInserted(int from, int to);

private:
	boost::shared_ptr<FeedItemData> data_;
	
	static QIcon sIcon;
};

#endif
