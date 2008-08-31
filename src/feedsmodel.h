#ifndef FEEDS_MODEL_H
#define FEEDS_MODEL_H

#include "feeditem.h"
#include "rootitem.h"
#include "subscriptionlist.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <QAbstractItemModel>
#include <QMap>
#include <QTimer>
#include <QUrl>

class AtomFeed;
class ReaderApi;
class TreeItem;
class Database;
class AllItems;
class FolderItem;

using boost::shared_ptr;
using boost::scoped_ptr;
using boost::weak_ptr;

// A tree model representing all the subscriptions and associated tags/folders.
class FeedsModel : public QAbstractItemModel {
	Q_OBJECT
public:
	FeedsModel(QObject* parent = 0);
	~FeedsModel();

	// Whether there is more data we can get from Google.
	QVariant data(const QModelIndex& index, int role) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	virtual Qt::DropActions supportedDropActions() const { return Qt::CopyAction | Qt::MoveAction; }
	virtual QStringList mimeTypes() const;
	virtual QMimeData* mimeData(const QModelIndexList& indices) const;
	virtual bool dropMimeData(const QMimeData* data,
		Qt::DropAction action, int row, int column, const QModelIndex& parent);

	TreeItem* root();
	QAbstractItemModel* getEntries(const QModelIndex& index) const;

	void getCategory(const QString& category, const QString& continuation = "");
	
	void load();
	void save();

private slots:
	void loggedIn();
	void subscriptionListArrived(SubscriptionList list);
	void dataDestroyed(QObject*);
	void googleAccountChanged();
	void categoryFeedArrived(const AtomFeed&);
	void freshFeedArrived(const AtomFeed&);

public slots:
	void fetchMore();

private:
	void addFeed(FeedItemData* data, bool update = true);
	
	RootItem root_;
	AllItems* all_items_;
	ReaderApi* api_;
	QMap<QString, weak_ptr<FeedItemData> > id_mappings_;
	QMap<QString, FolderItem*> folder_mappings_;
	scoped_ptr<Database> database_;

	bool deleting_;

	QTimer refresh_timer_;
};


#endif
