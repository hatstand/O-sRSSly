#include "folderitem.h"
#include "readerapi.h"
#include "database.h"

#include <QDebug>
#include <QSqlQuery>

QIcon FolderItem::sIcon;

FolderItem::FolderItem(TreeItem* parent, const QString& id, const QString& name, ReaderApi* api, Database* db)
	: TreeItem(parent, name),
	  api_(api),
	  db_(db)
{
	id_ = id;
	
	if (sIcon.isNull())
		sIcon = QIcon(":directory.png");
}

void FolderItem::save()
{
	QString query("REPLACE INTO Tag (id, title) VALUES (:id, :title)");
	QList<QVariant> bind_values;
	bind_values << id_;
	bind_values << title_;
	db_->pushQuery(query, bind_values);
}

QVariant FolderItem::data(const QModelIndex& index, int role) const {
	QModelIndex item = getItem(index);
	if (!item.isValid())
		return QVariant();

	// Return the feed name
	if (index.column() == TreeItem::Column_Preview && role == Qt::DisplayRole) {
		QModelIndex get_title = item.model()->index(item.row(), TreeItem::Column_FeedName, item.parent());
		return item.model()->data(get_title, role);
	}
	
	return item.model()->data(item, role);
}

bool FolderItem::setData(const QModelIndex& index, const QVariant& value, int role) {
	QModelIndex item = getItem(index);

	if (!item.isValid())
		return false;
	
	return const_cast<QAbstractItemModel*>(item.model())->setData(item, value, role);
}

int FolderItem::rowCount(const QModelIndex& parent) const {
	int rows = 0;
	foreach (TreeItem* item, children_) {
		rows += item->rowCount(parent);
	}

	return rows;
}

void FolderItem::fetchMore(const QModelIndex& parent) {
	Q_UNUSED(parent);
	api_->getCategory(id_, continuation_);
}

QModelIndex FolderItem::getItem(const QModelIndex& index) const {
	int rows = 0;

	QList<TreeItem*>::const_iterator it = children_.begin();

	while (index.row() >= (rows += (*it)->rowCount(index))) {
		++it;
	}

	if (it == children_.end()) {
		qWarning() << "Reached end";
		return QModelIndex();
	}

	int row_index = index.row() - rows + (*it)->rowCount(index);

	int safe_column = index.column();
	if (safe_column >= (*it)->columnCount(index))
		safe_column = (*it)->columnCount(index) - 1;

	return (*it)->index(row_index, safe_column);
}

const AtomEntry& FolderItem::entry(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->entry(item);
}

// TODO: Fix this shit
/*void FolderItem::childChanged(TreeItem* sender, const QModelIndex& top_left, const QModelIndex& bottom_right) {
	TreeItem::childChanged(top_left, bottom_right);
	
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
}*/
