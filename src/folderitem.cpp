#include "folderitem.h"
#include "readerapi.h"

#include <QDebug>
#include <QSqlQuery>

FolderItem::FolderItem(TreeItem* parent, const QString& id, const QString& name, ReaderApi* api)
	: TreeItem(parent, name),
	  api_(api)
{
	id_ = id;
}

FolderItem::FolderItem(TreeItem* parent, const QSqlQuery& query, ReaderApi* api)
	: TreeItem(parent),
	  api_(api)
{
	rowid_ = query.value(0).toLongLong();
	id_ = query.value(1).toString();
	title_ = query.value(2).toString();
	
	qDebug() << "Loaded folder" << rowid_ << id_ << title_;
}

void FolderItem::save()
{
	QSqlQuery query;
	if (rowid_ == -1)
	{
		query.prepare("INSERT INTO Tag (id, title) VALUES (:id, :title)");
		query.bindValue(":id", id_);
		query.bindValue(":title", title_);
		query.exec();
		
		rowid_ = query.lastInsertId().toLongLong();
	}
	else
	{
		query.prepare("UPDATE Tag SET id=:id, title=:title WHERE ROWID=:rowid");
		query.bindValue(":id", id_);
		query.bindValue(":title", title_);
		query.bindValue(":rowid", rowid_);
		query.exec();
	}
}

int FolderItem::columnCount() const {
	return 1;
}

QVariant FolderItem::data(const QModelIndex& index, int role) const {
	QModelIndex item = getItem(index);
	if (!item.isValid())
		return QVariant();

	// Return the feed name
	if (index.column() == 3 && role == Qt::DisplayRole)
		return static_cast<const TreeItem*>(item.model())->title();
	
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
	api_->getCategory(id_, continuation_);
}

QString FolderItem::summary(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	if (!item.isValid())
		return QString();

	return static_cast<const TreeItem*>(item.model())->summary(item);
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

void FolderItem::setRead(const QModelIndex& index) {
	if (!index.isValid())
		return;
	
	QModelIndex i = getItem(index);

	if (!i.isValid())
		return;

	TreeItem* item = const_cast<TreeItem*>(static_cast<const TreeItem*>(i.model()));
	item->setRead(i);
}

QString FolderItem::real_id(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	if (item.isValid()) {
		return static_cast<const TreeItem*>(item.model())->real_id(item);
	} else {
		return id_;
	}
}

QString FolderItem::content(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->content(item);
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
