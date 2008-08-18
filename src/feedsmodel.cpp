#include "feedsmodel.h"
#include "readerapi.h"

#include <QRegExp>

FeedsModel::FeedsModel(QObject* parent)
	: QAbstractItemModel(parent), root_(0, "Root-Id", "/"),
		api_(new ReaderApi("timetabletest2@googlemail.com", "timetabletestpassword", this)) {

	connect(api_, SIGNAL(loggedIn()), SLOT(loggedIn()));
	connect(api_, SIGNAL(subscriptionListArrived(SubscriptionList)),
		SLOT(subscriptionListArrived(SubscriptionList)));
	connect(api_, SIGNAL(subscriptionArrived(AtomFeed*)), SLOT(subscriptionUpdated(AtomFeed*)));

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

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
		FeedItem::Data* d = new FeedItem::Data(s);

		id_mappings_.insert(s.id(), d);
		api_->getSubscription(s);

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

void FeedsModel::update(const QModelIndex& index) {
	if (!index.isValid())
		return;
	
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (item->rtti() != TreeItem::Feed)
		return;
	
	FeedItem* feed = static_cast<FeedItem*>(item);
	api_->getSubscription(feed->subscription());
}

void FeedsModel::subscriptionUpdated(AtomFeed* feed) {
	QString atom_id(feed->id());
	atom_id.remove(QRegExp("^tag:google.com,2005:reader/"));

	qDebug() << "Update for..." << atom_id;

	QMap<QString, FeedItem::Data*>::iterator it = id_mappings_.find(atom_id);
	if (it != id_mappings_.end()) {
		(*it)->update(*feed);
	}

	delete feed;
}

bool FeedsModel::canFetchMore(const QModelIndex& index) const {
	if (!index.isValid())
		return false;
	
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (item->rtti() == TreeItem::Feed)
		return true;

	return false;
}

void FeedsModel::fetchMore(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	if (!index.isValid())
		return;

	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	if (item->rtti() == TreeItem::Feed) {
		FeedItem* feed = static_cast<FeedItem*>(index.internalPointer());
		api_->getSubscription(feed->subscription());
	}
}

QAbstractItemModel* FeedsModel::getEntries(const QModelIndex& index) {
	if (!index.isValid())
		return 0;
	
	TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
	return item;
}

void FeedsModel::setRead(const AtomEntry& e) {
	if (!e.read)
		api_->setRead(e);
}
