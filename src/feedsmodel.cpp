#include "feedsmodel.h"
#include "readerapi.h"

#include <QMimeData>
#include <QRegExp>
#include <QStringList>

FeedsModel::FeedsModel(QObject* parent)
	: QAbstractItemModel(parent), root_(0, "Root-Id", "/"),
		api_(new ReaderApi("timetabletest2@googlemail.com", "timetabletestpassword", this)) {

	connect(api_, SIGNAL(loggedIn()), SLOT(loggedIn()));
	connect(api_, SIGNAL(subscriptionListArrived(SubscriptionList)),
		SLOT(subscriptionListArrived(SubscriptionList)));

	api_->login();
}

FeedsModel::~FeedsModel() {
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
		return 0;

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
		FeedItemData* d = new FeedItemData(s, api_);

		id_mappings_.insert(s.id(), d);
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
			
			stream << text;
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
	QStringList new_items;

	while (!stream.atEnd()) {
		QString text;
		stream >> text;
		new_items << text;
	}

	if (parent.isValid()) {
		TreeItem* item = static_cast<TreeItem*>(parent.internalPointer());
		// Climb up the tree until we find a folder.
		while (item->rtti() != TreeItem::Folder) {
			item = item->parent();
		}

		foreach (QString s, new_items) {
			qDebug() << "Adding:" << s << "to:" << item->id();
			if (s.startsWith("feed")) {
				QMap<QString, FeedItemData*>::const_iterator it = id_mappings_.find(s);
				if (it != id_mappings_.end()) {
					beginInsertRows(parent, item->childCount(), item->childCount());
					FeedItem* new_feed = new FeedItem(item, *it);
					item->appendChild(new_feed);
					new_feed->addCategory(qMakePair(item->id(), item->title()));
					endInsertRows();	
				}
			}
		}
	}

	return true;
}
