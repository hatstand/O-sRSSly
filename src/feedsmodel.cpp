#include "feedsmodel.h"
#include "readerapi.h"

#include <QMimeData>
#include <QRegExp>
#include <QStringList>

FeedsModel::FeedsModel(QObject* parent)
	: QAbstractItemModel(parent), root_(0, "Root-Id", "/"),
		api_(new ReaderApi("timetabletest2@googlemail.com", "timetabletestpassword", this)),
		deleting_(false) {

	connect(api_, SIGNAL(loggedIn()), SLOT(loggedIn()));
	connect(api_, SIGNAL(subscriptionListArrived(SubscriptionList)),
		SLOT(subscriptionListArrived(SubscriptionList)));

	api_->login();
}

FeedsModel::~FeedsModel() {
	deleting_ = true;
}

QVariant FeedsModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

	return item->data(index.column());
}

int FeedsModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return root_.columnCount();
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
		parent_item = const_cast<FolderItem*>(&root_);
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
		parent_item = const_cast<FolderItem*>(&root_);
	else
		parent_item = static_cast<TreeItem*>(parent.internalPointer());

	return parent_item->childCount();
}

TreeItem* FeedsModel::root() {
	return &root_;
}

void FeedsModel::loggedIn() {
	api_->getSubscriptionList();
}

void FeedsModel::subscriptionListArrived(SubscriptionList list) {
	qDebug() << __PRETTY_FUNCTION__;

	foreach (const Subscription& s, list.subscriptions()) {
		qDebug() << "Adding..." << s.title();

		// Create actual shared data.
		shared_ptr<FeedItemData> d(new FeedItemData(s, api_));

		id_mappings_.insert(s.id(), d);
		connect(d.get(), SIGNAL(destroyed(QObject*)), SLOT(dataDestroyed(QObject*)));
		d->update();

		// If it has no categories then insert at root level.
		if (s.categories().isEmpty()) {
			FeedItem* feed = new FeedItem(&root_, d);
			root_.appendChild(feed);

			continue;
		}

		typedef QPair<QString, QString> Category;
		// Insert a FeedItem under every category.
		foreach (const Category& c, s.categories()) {
			qDebug() << "Adding category..." << c.first << c.second;
			TreeItem* parent = 0;
			if (folder_mappings_.contains(c.first)) {
				parent = folder_mappings_[c.first];
			} else {
				FolderItem* f = new FolderItem(&root_, c.first, c.second);
				root_.appendChild(f);
				folder_mappings_.insert(c.first, f);
				parent = f;
			}

			FeedItem* feed = new FeedItem(parent, d);
			parent->appendChild(feed);
		}

	}

	// Notify the view that the model has changed.
	reset();
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
		while (item->rtti() != TreeItem::Folder) {
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
					item->appendChild(new_feed);
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
					FeedItem* feed = new FeedItem(&root_, data);
					root_.appendChild(feed);
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
