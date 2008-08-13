#ifndef FEEDS_MODEL_H
#define FEEDS_MODEL_H

#include "feeditem.h"
#include "folderitem.h"
#include "subscriptionlist.h"

#include <boost/shared_ptr.hpp>

#include <QAbstractItemModel>
#include <QMap>
#include <QUrl>

class AtomFeed;
class ReaderApi;
class TreeItem;

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
