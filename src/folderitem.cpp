#include "folderitem.h"

#include <QDebug>

FolderItem::FolderItem(TreeItem* parent, const QString& id, const QString& name)
	: TreeItem(parent, name) {
	id_ = id;
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

void FolderItem::fetchMore(const QModelIndex& parent) {
	foreach (TreeItem* item, children_) {
		item->fetchMore(parent);
	}
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

const AtomEntry& FolderItem::entry(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->entry(item);
}

void FolderItem::setRead(const QModelIndex& index) {
	if (!index.isValid())
		return;
	
	QModelIndex i = getItem(index);

	TreeItem* item = const_cast<TreeItem*>(static_cast<const TreeItem*>(i.model()));
	item->setRead(i);
}

void FolderItem::childChanged(const QModelIndex& top_left, const QModelIndex& bottom_right) {
	TreeItem* item = static_cast<TreeItem*>(sender());

	int rows = 0;
	for (QList<TreeItem*>::const_iterator it = children_.begin(); it != children_.end(); ++it) {
		if (*it == item)
			break;

		rows += (*it)->rowCount(QModelIndex());
	}

	QModelIndex top_left_new = createIndex(rows + top_left.row(), top_left.column());
	QModelIndex bottom_right_new = createIndex(rows + bottom_right.row(), bottom_right.column());

	emit dataChanged(top_left_new, bottom_right_new);
}
