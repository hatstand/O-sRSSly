#include "treeitem.h"

#include <QDebug>

TreeItem::TreeItem(TreeItem* parent, const QString& title)
	: parent_(parent),
	  changed_proxy_(NULL),
	  fail_prevention_(false),
	  title_(title),
	  rowid_(-1)
{
	if (parent) {
		connect(this, SIGNAL(modelReset()), parent, SLOT(childReset()));
		connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
			parent, SLOT(childRowsInserted(const QModelIndex&, int, int)));
	}
}

TreeItem::~TreeItem() {
	clear();
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


int TreeItem::columnCount(const QModelIndex& parent) const {
	return 6;
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
}

void TreeItem::childDestroyed(QObject* object) {
	children_.removeAll(static_cast<TreeItem*>(object));
}

void TreeItem::childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end) {
	if (fail_prevention_)
		return;
	
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
