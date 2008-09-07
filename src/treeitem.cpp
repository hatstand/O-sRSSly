#include "treeitem.h"
#include "feeditem.h"
#include "feedsmodel.h"

#include <QDebug>

TreeItem::TreeItem(TreeItem* parent, const QString& title) {
	init(parent, title, parent->feedsModel());
	
	parent->appendChild(this);
	connect(this, SIGNAL(modelReset()), parent, SLOT(childReset()));
	connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
		parent, SLOT(childRowsInserted(const QModelIndex&, int, int)));
	
	connect(this, SIGNAL(unreadCountChanged(const QModelIndex&)), feeds_model_, SLOT(unreadCountChanged(const QModelIndex&)));
}

TreeItem::TreeItem(FeedsModel* model, const QString& title) {
	init(NULL, title, model);
}

TreeItem::~TreeItem() {
	deleting_ = true;
	clear();
}

void TreeItem::init(TreeItem* parent, const QString& title, FeedsModel* model) {
	parent_ = parent;
	title_ = title;
	feeds_model_ = model;
	changed_proxy_ = NULL;
	fail_prevention_ = false;
	rowid_ = -1;
	unread_count_= -1;
	deleting_ = false;
}

void TreeItem::clear() {
	qDeleteAll(children_);
	children_.clear();
}

void TreeItem::appendChild(TreeItem* child) {
	children_ << child;
	connect(child, SIGNAL(destroyed(QObject*)), SLOT(childDestroyed(QObject*)));
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

int TreeItem::row() const {
	if (parent_)
		return parent_->children_.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

QVariant TreeItem::data(int column, int role) const {
	switch (column)
	{
	case FeedItem::Column_Title:
		if (role == Qt::DecorationRole)
			return icon();
		else
			return title_;
	case FeedItem::Column_UnreadCount:
		return unreadCount();
	default:
		return QVariant();
	}
}

QVariant TreeItem::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();
	
	switch (section) {
		case 0:
			return "Title";
		case 1:
			return "Read/Unread";
		case 2:
			return "Date";

		default:
			return QVariant();
	}

	return QVariant();
}

void TreeItem::childRowsInserted(const QModelIndex& parent, int start, int end) {
	childRowsInserted(static_cast<TreeItem*>(sender()), parent, start, end);
}

void TreeItem::childReset() {
	reset();
	invalidateUnreadCount();
}

void TreeItem::childDestroyed(QObject* object) {
	if (!deleting_) {
		children_.removeAll(static_cast<TreeItem*>(object));
		invalidateUnreadCount();
	}
}

void TreeItem::childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end) {
	if (fail_prevention_)
		return;

	invalidateUnreadCount();
	
	fail_prevention_ = true;
	if (changed_proxy_)
		changed_proxy_->childRowsInserted(sender, parent, start, end);
	else if (parent_)
		parent_->childRowsInserted(sender, parent, start, end);
	fail_prevention_ = false;
}

QList<TreeItem*> TreeItem::allChildren() const {
	QList<TreeItem*> ret;
	
	foreach (TreeItem* child, children_) {
		ret << child;
		ret << child->allChildren();
	}
	
	return ret;
}

int TreeItem::unreadCount() const {
	if (unread_count_ == -1) {
		int& unreadRef = const_cast<TreeItem*>(this)->unread_count_;
		unreadRef = 0;
		
		const int total = rowCount(QModelIndex());
		for (int i=0 ; i<total; ++i) {
			if (!data(index(i, FeedItem::Column_Read)).toBool())
				unreadRef++;
		}
	}
	
	return unread_count_;
}

void TreeItem::invalidateUnreadCount() {
	if (fail_prevention_)
		return;
	
	fail_prevention_ = true;
	if (parent_)
		parent_->invalidateUnreadCount();
	if (changed_proxy_)
		changed_proxy_->invalidateUnreadCount();
	fail_prevention_ = false;
	
	if (unread_count_ == -1)
		return;
	
	unread_count_ = -1;
	emit unreadCountChanged(indexInFeedsModel());
}

void TreeItem::decrementUnreadCount() {
	if (fail_prevention_)
		return;
	
	fail_prevention_ = true;
	if (parent_)
		parent_->decrementUnreadCount();
	if (changed_proxy_)
		changed_proxy_->decrementUnreadCount();
	fail_prevention_ = false;
	
	if (unread_count_ == -1)
		return;
	
	unread_count_--;
	emit unreadCountChanged(indexInFeedsModel());
}

QModelIndex TreeItem::indexInFeedsModel() const {
	QModelIndex parentIndex;
	if (parent_)
		parentIndex = parent_->indexInFeedsModel();
	
	return feeds_model_->index(row(), 0, parentIndex);
}
