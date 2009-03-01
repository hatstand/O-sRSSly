#include "feedsmodel.h"
#include "readerapi.h"
#include "settings.h"
#include "database.h"
#include "allitems.h"
#include "folderitem.h"

#include <QApplication>
#include <QMimeData>
#include <QMutexLocker>
#include <QRegExp>
#include <QStringList>

#include <boost/bind.hpp>

using boost::bind;
using boost::shared_ptr;
using boost::weak_ptr;

const QString FeedsModel::kSharedFolder = "OsrsslyShared";

FeedsModel::FeedsModel(Database* database, ReaderApi* api, QObject* parent)
	: QAbstractItemModel(parent),
	  root_(this),
	  all_items_(NULL),
	  api_(api),
	  database_(database),
	  deleting_(false),
	  refresh_timer_(this)
{
	connect(Settings::instance(), SIGNAL(googleAccountChanged()), SLOT(googleAccountChanged()));
	connect(&refresh_timer_, SIGNAL(timeout()), SLOT(fetchMore()));

	connect(this, SIGNAL(doLater(VoidFunction)), this, SLOT(doNow(VoidFunction)), Qt::QueuedConnection);
	
	connect(this, SIGNAL(newUnreadItems(int)), qApp, SLOT(setUnreadItems(int)));

	connect(api_, SIGNAL(progressChanged(int, int)), SIGNAL(progressChanged(int, int)));
	connect(api_, SIGNAL(loggedIn(bool)), SLOT(loggedIn(bool)));
	connect(api_, SIGNAL(subscriptionListArrived(SubscriptionList)),
		SLOT(subscriptionListArrived(SubscriptionList)));
	connect(api_, SIGNAL(categoryArrived(const AtomFeed&)),
		SLOT(categoryFeedArrived(const AtomFeed&)));
	connect(api_, SIGNAL(freshArrived(const AtomFeed&)),
		SLOT(freshFeedArrived(const AtomFeed&)));
	connect(api_, SIGNAL(friendsArrived(const AtomFeed&)),
		SLOT(freshFeedArrived(const AtomFeed&)));

	initialiseModel();
}

FeedsModel::~FeedsModel() {
	deleting_ = true;
	// Try & stop the db thread.
	// Wait 30s. This can take a long time as the db won't quit until it's written
	// everything in the queue.
	database_->stop();
	//database_->wait(30000);
}

void FeedsModel::initialiseModel() {
	all_items_ = new AllItems(&root_, api_);
	root_.installChangedProxy(all_items_);

	FolderItem* friends = new FolderItem(&root_, kSharedFolder, "Shared Items", api_, database_);
	folder_mappings_.insert(kSharedFolder, friends);

	// Notify views.
	reset();
}

void FeedsModel::googleAccountChanged() {
	// Clear any existing items in the model

	if (!api_->isLoggedIn()) {
		qDebug() << __PRETTY_FUNCTION__ << "not logged in yet, doing it now";
		api_->login(Settings::instance()->googleUsername(), Settings::instance()->googlePassword());

		root_.clear();
		{
			QMutexLocker locker(&mutex_);
			folder_mappings_.clear();
			id_mappings_.clear();
		}

		// Load stuff from db.
		// This has to happen after we create the ReaderApi.
		/*database_->start();
		load();*/

		// Add smart folders to root again.
		// These get deleted by root_.clear()
		initialiseModel();
	}
}

QVariant FeedsModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

	return item->data(index.column(), role);
}

int FeedsModel::columnCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return 17;
}

Qt::ItemFlags FeedsModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant FeedsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return "Title";

	return QVariant();
}

QModelIndex FeedsModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem* parent_item;
	if (!parent.isValid())
		parent_item = const_cast<RootItem*>(&root_);
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());

	TreeItem* child_item = parent_item->child(row);
	if (child_item)
		return createIndex(row, column, child_item);
	else
		return QModelIndex();
}

QModelIndex FeedsModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	TreeItem* child_item = static_cast<TreeItem*>(index.internalPointer());
	TreeItem* parent_item = child_item->parent();

	if (parent_item == &root_)
		return QModelIndex();

	if (!parent_item)
		return QModelIndex();

	return createIndex(parent_item->row(), 0, parent_item);
}

int FeedsModel::rowCount(const QModelIndex& parent) const {
	TreeItem* parent_item;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parent_item = const_cast<RootItem*>(&root_);
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());

	return parent_item->childCount();
}

TreeItem* FeedsModel::root() {
	return &root_;
}

void FeedsModel::loggedIn(bool success) {
	if (success) {
		api_->getSubscriptionList();

		// Refresh feeds every 5 minutes.
		refresh_timer_.start(5*60*1000);
	} else {
		qWarning() << __PRETTY_FUNCTION__ << "Login failed";
	}
}

void FeedsModel::subscriptionListArrived(SubscriptionList list) {
	qDebug() << __PRETTY_FUNCTION__;

	api_->getFresh();
	api_->getFriends();

	foreach (Subscription* s, list.subscriptions()) {
		qDebug() << "Adding..." << s->title();
		
		{
			QMutexLocker locker(&mutex_);
			if (id_mappings_.contains(s->id()))
			{
				shared_ptr<FeedItemData> d(id_mappings_[s->id()]);
				d.get()->update();
				continue;
			}
		}

		addFeed(new FeedItemData(s, api_, database_));
	}

	// Notify the view that the model has changed.
	//reset();

	//api_->search("cheese");
}

void FeedsModel::addFeed(FeedItemData* data, bool update)
{
	// TODO: make this more fine-grained?
	QMutexLocker locker(&mutex_);
	if (id_mappings_.contains(data->subscription().id())) {
		delete data;
		return;
	}
	shared_ptr<FeedItemData> d(data);

	id_mappings_.insert(d.get()->subscription().id(), d);
	connect(d.get(), SIGNAL(destroyed(QObject*)), SLOT(dataDestroyed(QObject*)));

	connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), d.get(), SLOT(update(const AtomFeed&)));
	
	if (update)
		d->update();

	// If it has no categories then insert at root level.
	if (d.get()->subscription().categories().isEmpty()) {
		beginInsertRows(QModelIndex(), root_.childCount(), root_.childCount());
		new FeedItem(&root_, d);
		endInsertRows();
		return;
	}

	// Insert a FeedItem under every category.
	foreach (const Category& c, d.get()->subscription().categories()) {
		qDebug() << "Adding category..." << c.first << c.second;
		TreeItem* parent = 0;
		if (folder_mappings_.contains(c.first)) {
			parent = folder_mappings_[c.first];
		} else {
			beginInsertRows(QModelIndex(), root_.childCount(), root_.childCount());
			FolderItem* f = new FolderItem(&root_, c.first, c.second, api_, database_);
			f->save();
			folder_mappings_.insert(c.first, f);
			endInsertRows();
			parent = f;
		}

		beginInsertRows(createIndex(0, 0, parent), parent->childCount(), parent->childCount());
		new FeedItem(parent, d);
		endInsertRows();
	}
	
	all_items_->invalidateFeedCache();
}

QAbstractItemModel* FeedsModel::getEntries(const QModelIndex& index) const {
	if (!index.isValid())
		return 0;
	
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	return item;
}

QStringList FeedsModel::mimeTypes() const {
	QStringList types;
	types << "application/x-feeder";
	return types;
}

QMimeData* FeedsModel::mimeData(const QModelIndexList& indices) const {
	QMimeData* mime_data = new QMimeData();
	QByteArray encoded_data;

	QDataStream stream(&encoded_data, QIODevice::WriteOnly);

	foreach (const QModelIndex& index, indices) {
		if (index.isValid()) {
			TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
			QString text = item->id();
			if (text.isEmpty())
				continue;

			QString category = item->parent()->id();
			
			stream << text << category;
		}
	}

	mime_data->setData("application/x-feeder", encoded_data);
	return mime_data;
}

bool FeedsModel::dropMimeData(const QMimeData* data,
	Qt::DropAction action, int row, int column, const QModelIndex& parent) {
	Q_UNUSED(row);
	Q_UNUSED(column);
	qDebug() << __PRETTY_FUNCTION__ << action;

	if (action == Qt::IgnoreAction)
		return true;
	
	if (!data->hasFormat("application/x-feeder"))
		return false;

	QByteArray encoded_data = data->data("application/x-feeder");
	QDataStream stream(&encoded_data, QIODevice::ReadOnly);
	// Map from subscription id -> category
	QMap<QString,QString> new_items;

	while (!stream.atEnd()) {
		QString text, category;
		stream >> text;
		stream >> category;
		new_items.insert(text, category);
	}

	if (parent.isValid()) {
		// Dropped on an actual TreeItem.
		TreeItem* item = static_cast<TreeItem*>(parent.internalPointer());
		// Climb up the tree until we find a folder.
		while (qobject_cast<FolderItem*>(item) == NULL) {
			item = item->parent();
		}

		// Add each item to the category.
		QMap<QString,QString>::const_iterator it = new_items.begin();
		for (; it != new_items.end(); ++it) {
			qDebug() << "Adding:" << it.key() << "to:" << item->id();
			// Make sure it's really a feed and not a folder.
			if (it.key().startsWith("feed")) {
				// Find the appropriate data.
				QMap<QString, weak_ptr<FeedItemData> >::const_iterator jt = id_mappings_.find(it.key());
				if (jt != id_mappings_.end()) {
					// Insert the new FeedItem into the tree.
					QModelIndex index = createIndex(item->row(), 0, item);
					beginInsertRows(index, item->childCount(), item->childCount());
					FeedItem* new_feed = new FeedItem(item, shared_ptr<FeedItemData>(*jt));
					new_feed->addCategory(qMakePair(item->id(), item->title()));
					endInsertRows();	
				}

				// Remove from root.
				if (it.value() == "Root-Id") {
					// This was dropped from the root. We should remove the root FeedItem.
					for (int i = 0; i < root_.childCount(); ++i) {
						if (root_.child(i)->id() == it.key()) {
							// Remove FeedItem
							QModelIndex index = createIndex(root_.row(), 0, &root_);
							//beginRemoveRows(index, i, i);
							delete root_.child(i);
							//endRemoveRows();
							reset();

							break;
						}
					}
				}
			}
		}
	} else {
		// Dropped outside of tree -> Remove from the folder.
		QMap<QString,QString>::const_iterator it = new_items.begin();
		for (; it != new_items.end(); ++it) {
			QMap<QString, weak_ptr<FeedItemData> >::const_iterator jt = id_mappings_.find(it.key());
			if (jt != id_mappings_.end()) {
				qDebug() << "Removing category...";
				QMap<QString,FolderItem*>::const_iterator kt = folder_mappings_.find(it.value());
				if (kt == folder_mappings_.end())
					continue;

				FolderItem* folder = *kt;
				int i = 0;
				TreeItem* child = 0;
				for (; i < folder->childCount(); ++i) {
					if (folder->child(i)->id() == it.key()) {
						child = folder->child(i);
						break;
					}
				}

				QModelIndex index = createIndex(folder->row(), 0, folder);
				//beginRemoveRows(index, i, i);
				// Temporarily grab a reference to the FeedItemData
				shared_ptr<FeedItemData> data(jt.value());
				data->removeCategory(it.value());
				delete child;
				//endRemoveRows();
				reset();

				// If we have the last reference, then we should add the Feed to the root of the tree.
				if (data.unique()) {
					qDebug() << "Adding feed to root as removed from all categories";
					QModelIndex index = createIndex(root_.row(), 0, &root_);
					beginInsertRows(index, root_.childCount(), root_.childCount());
					new FeedItem(&root_, data);
					endInsertRows();
				}
			}
		}
	}

	return true;
}

void FeedsModel::dataDestroyed(QObject* object) {
	if (deleting_)
		return;

	QMap<QString, weak_ptr<FeedItemData> >::iterator it = id_mappings_.begin();
	for (; it != id_mappings_.end(); ++it) {
		if (it.value().lock().get() == static_cast<FeedItemData*>(object)) {
			id_mappings_.erase(it);
			return;
		}
	}
}

void FeedsModel::load() {
	// Load tags & feeds.
	// Callback will be called from different thread. CAREFUL :-)
	database_->pushQuery("SELECT id, title FROM Tag", bind(&FeedsModel::folderItemsLoaded, this, _1));
	database_->pushQuery("SELECT id, title, sortId FROM Feed", bind(&FeedsModel::feedItemsLoaded, this, _1));
}

void FeedsModel::folderItemsLoaded(const QSqlQuery& query) {
	qDebug() << __PRETTY_FUNCTION__;

	QSqlQuery mutable_query(query);
	while (mutable_query.next()) {
		QString id = mutable_query.value(0).toString();
		QString name = mutable_query.value(1).toString();
		// We have to do it in the gui thread :-(
		emit doLater(bind(&FeedsModel::createFolderItem, this, id, name));
	}
}

void FeedsModel::doNow(VoidFunction func) {
	func();
}

void FeedsModel::createFolderItem(const QString& id, const QString& name) {
	qDebug() << __PRETTY_FUNCTION__;

	QMutexLocker locker(&mutex_);
	if (!folder_mappings_.contains(id)) {
		FolderItem* f = new FolderItem(&root_, id, name, api_, database_);
		folder_mappings_.insert(f->id(), f);
	}
}

void FeedsModel::feedItemsLoaded(const QSqlQuery& query) {
	qDebug() << __PRETTY_FUNCTION__;

	QSqlQuery mutable_query(query);
	while (mutable_query.next()) {
		FeedItemData* data = new FeedItemData(mutable_query, api_, database_);
		if (data->subscription().id().startsWith("user"))
			data->addCategory(Category(kSharedFolder, "Shared Items"), false);
		
		database_->pushQuery(
			"SELECT Tag.id, Tag.title FROM Tag "
			"INNER JOIN FeedTagMap ON Tag.id=FeedTagMap.tagId "
			"WHERE FeedTagMap.feedId=?",
			QList<QVariant>() << data->subscription().id(),
			bind(&FeedsModel::categoriesLoaded, this, _1, data));
	}
}

void FeedsModel::categoriesLoaded(const QSqlQuery& query, FeedItemData* data) {
	QSqlQuery mutable_query(query);
	while (mutable_query.next()) {
		Category category(mutable_query.value(0).toString(), mutable_query.value(1).toString());
		data->addCategory(category, false);
	}

	data->moveToThread(QApplication::instance()->thread());

	emit doLater(bind(&FeedsModel::addFeed, this, data, false));
}

void FeedsModel::save() {
	
}

void FeedsModel::categoryFeedArrived(const AtomFeed& feed) {
	qDebug() << __PRETTY_FUNCTION__;
	QMap<QString, FolderItem*>::const_iterator it = folder_mappings_.find(feed.id());
	if (it == folder_mappings_.end())
		return;

	it.value()->setContinuation(feed.continuation());

	for (AtomFeed::AtomList::const_iterator kt = feed.entries().begin(); kt != feed.entries().end(); ++kt) {
		QMap<QString, weak_ptr<FeedItemData> >::const_iterator jt = id_mappings_.find(kt->id);
		if (jt == id_mappings_.end())
			continue;

		// Grab reference.
		shared_ptr<FeedItemData> data(jt.value());
		data->update(*kt, true);
	}
}

void FeedsModel::freshFeedArrived(const AtomFeed& feed) {
	qDebug() << __PRETTY_FUNCTION__ << "Size:" << feed.entries().size();

	int new_unread = 0;
	for (AtomFeed::AtomList::const_iterator it = feed.entries().begin(); it != feed.entries().end(); ++it) {
		QMap<QString, weak_ptr<FeedItemData> >::const_iterator jt = id_mappings_.find(it->source);
		if (jt == id_mappings_.end()) {
			// Either a stray entry or a `Shared Item'
			if (it->source.startsWith("user")) {
				// Must be a shared item
				qDebug() << "Adding shared items for:" << it->shared_by;
				Subscription* sub = new Subscription(it->source, it->shared_by);
				sub->addCategory(Category(kSharedFolder, "Shared Items"));
				FeedItemData* data = new FeedItemData(sub, api_, database_);
				new_unread += data->update(*it, true);
				addFeed(data, false);
			}
			continue;
		}

		// Grab reference.
		shared_ptr<FeedItemData> data(jt.value());
		new_unread += data->update(*it, true);
	}
	
	emit newUnreadItems(new_unread);
}

void FeedsModel::fetchMore() {
	api_->getFresh();
}

void FeedsModel::unreadCountChanged(const QModelIndex& index) {
	emit dataChanged(index, index.sibling(index.row(), 3));
}

int FeedsModel::unread() const {
	int unread = 0;
	for (QMap<QString,weak_ptr<FeedItemData> >::const_iterator it = id_mappings_.begin(); it != id_mappings_.end(); ++it) {
		unread += it.value().lock()->unread();
	}

	return unread;
}

// Hack.
void FeedsModel::forceReset() {
	reset();
}
