#ifndef FEEDS_MODEL_H
#define FEEDS_MODEL_H

#include "atomfeed.h"
#include "subscriptionlist.h"

#include <boost/shared_ptr.hpp>

#include <QAbstractItemModel>
#include <QMap>
#include <QModelIndex>
#include <QUrl>

class ReaderApi;

using boost::shared_ptr;

class TreeItem {
public:
	enum Type {
		Feed = 0,
		Folder
	};

	TreeItem(TreeItem* parent = 0, const QString& title = QString());

	void appendChild(TreeItem* child);
	TreeItem* child(int row);
	int childCount() const;
	virtual int columnCount() const = 0;
	int row() const;
	virtual QVariant data(int column) const;
	virtual Type rtti() const = 0;

	TreeItem* parent();

	QString title() const;

protected:
	TreeItem* parent_;
	QList<TreeItem*> children_;
	QString title_;
};

class FeedsModel;

class FeedItem : public TreeItem {
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

private:
	shared_ptr<Data> data_;
};

class FolderItem : public TreeItem {
public:
	FolderItem(TreeItem* parent, const QString& id, const QString& name);
	int columnCount() const;
	TreeItem::Type rtti() const { return TreeItem::Folder; }

private:
	QString id_;
};


class FeedsModel : public QAbstractItemModel {
	Q_OBJECT
public:
	FeedsModel(QObject* parent = 0);
	~FeedsModel();

	virtual bool canFetchMore(const QModelIndex& index) const;
	QVariant data(const QModelIndex& index, int role) const;
	virtual void fetchMore(const QModelIndex& index);
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	TreeItem* root();

	void update(const QModelIndex& index);
	QAbstractItemModel* getEntries(const QModelIndex& index);

private slots:
	void loggedIn();
	void subscriptionListArrived(SubscriptionList list);
	void subscriptionUpdated(AtomFeed* feed);

private:
	FolderItem root_;	
	ReaderApi* api_;
	QMap<QString, FeedItem::Data*> id_mappings_;
	QMap<QString, FolderItem*> folder_mappings_;
};


#endif
