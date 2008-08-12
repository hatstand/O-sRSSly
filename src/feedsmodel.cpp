#include "feedsmodel.h"
#include "readerapi.h"

TreeItem::TreeItem(TreeItem* parent, const QString& title)
	: parent_(parent), title_(title) {
	
}

void TreeItem::appendChild(TreeItem* child) {
	children_ << child;
}

TreeItem* TreeItem::child(int row) {
	return children_.at(row);
}

int TreeItem::childCount() const {
	return children_.size();
}

TreeItem* TreeItem::parent() {
	return parent_;
}

QString TreeItem::title() const {
	return title_;
}

int TreeItem::row() const {
	if (parent_)
		return parent_->children_.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

QVariant TreeItem::data(int column) const {
	if (column == 0)
		return title_;
	
	return QVariant();
}



FeedItem::FeedItem(TreeItem* parent, const QString& name, const QUrl& url)
	: TreeItem(parent, name), url_(url) {
}

int FeedItem::columnCount() const {
	return 2;
}

QVariant FeedItem::data(int column) const {
	switch (column) {
		case 0:
			return title_;
		case 1:
			return url_;
		default:
			return QVariant();
	}
}

FolderItem::FolderItem(TreeItem* parent, const QString& name)
	: TreeItem(parent, name) {
}

int FolderItem::columnCount() const {
	return 1;
}


FeedsModel::FeedsModel(QObject* parent)
	: QAbstractItemModel(parent), root_(0, "/"),
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

	FeedItem* begin = 0;
	FeedItem* end = 0;

	foreach (Subscription s, list.subscriptions()) {
		qDebug() << "Adding..." << s.title();
		FeedItem* feed = new FeedItem(&root_, s.title(),
			QUrl("http://www.google.com/reader/atom/" + s.id()));
		root_.appendChild(feed);

		if (!begin)
			begin = feed;

		end = feed;
	}

	emit reset();
}
