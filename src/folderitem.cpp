#include "folderitem.h"

#include <QDebug>

FolderItem::FolderItem(TreeItem* parent, const QString& id, const QString& name)
	: TreeItem(parent, name), id_(id) {
}

int FolderItem::columnCount() const {
	return 1;
}

QVariant FolderItem::data(const QModelIndex& index, int role) const {
	QModelIndex item = getItem(index);
	return item.model()->data(item, role);
}

int FolderItem::rowCount(const QModelIndex& parent) const {
	int rows = 0;
	foreach (TreeItem* item, children_) {
		rows += item->rowCount(parent);
	}

	return rows;
}


QString FolderItem::summary(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->summary(item);
}

QModelIndex FolderItem::getItem(const QModelIndex& index) const {
	int rows = 0;

	QList<TreeItem*>::const_iterator it = children_.begin();

	while (index.row() > (rows += (*it)->rowCount(index))) {
		++it;
	}

	if (it == children_.end()) {
		qWarning() << "Reached end";
		return QModelIndex();
	}

	int row_index = index.row() - rows + (*it)->rowCount(index);

	return (*it)->index(row_index, index.column());
}
