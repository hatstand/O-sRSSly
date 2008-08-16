#include "treeitem.h"

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


int TreeItem::columnCount(const QModelIndex& parent) const {
	return 2;
}

QVariant TreeItem::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();
	
	switch (section) {
		case 0:
			return "Title";
		case 1:
			return "Read/Unread";

		default:
			return QVariant();
	}

	return QVariant();
}
