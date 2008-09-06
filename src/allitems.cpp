#include "allitems.h"
#include "feeditem.h"
#include "readerapi.h"

#include <QDebug>

#define FOREACH_CHILD \
	QSet<QString> dups; \
	foreach (TreeItem* item, parent_->allChildren()) { \
		FeedItem* feed = qobject_cast<FeedItem*>(item); \
		if (!feed) \
			continue; \
		if (dups.contains(feed->subscription().id())) \
			continue; \
		dups << feed->subscription().id();

QIcon AllItems::sIcon;

AllItems::AllItems(TreeItem* parent, ReaderApi* api)
	: TreeItem(parent, "All Items"),
	  row_count_(-1),
	  api_(api)
{
	if (sIcon.isNull())
		sIcon = QIcon(":allitems.png");
}

QVariant AllItems::data(const QModelIndex& index, int role) const {
	QModelIndex item = getItem(index);
	if (!item.isValid())
		return QVariant();

	// Return the feed name
	if (index.column() == 3 && role == Qt::DisplayRole)
		return static_cast<const TreeItem*>(item.model())->title();
	
	return item.model()->data(item, role);
}

int AllItems::rowCount(const QModelIndex& parent) const {
	if (row_count_ == -1)
	{
		int& rowCountRef = const_cast<AllItems*>(this)->row_count_;
		rowCountRef = 0;
		
		FOREACH_CHILD
			rowCountRef += item->rowCount(QModelIndex());
		}
	}
	return row_count_;
}

void AllItems::fetchMore(const QModelIndex& parent) {
	api_->getFresh();
}

QString AllItems::summary(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	if (!item.isValid())
		return QString();

	return static_cast<const TreeItem*>(item.model())->summary(item);
}

QModelIndex AllItems::getItem(const QModelIndex& index) const {
	int rows = 0;
	
	FOREACH_CHILD
		int row_count = item->rowCount(QModelIndex());
		rows += row_count;

		if (rows <= index.row())
			continue;
		
		int row_index = index.row() - rows + row_count;
		
		int safe_column = index.column();
		if (safe_column >= item->columnCount(QModelIndex()))
			safe_column = item->columnCount(QModelIndex()) - 1;
	
		return item->index(row_index, safe_column);
	}

	qWarning() << "Reached end";
	return QModelIndex();
}

const AtomEntry& AllItems::entry(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->entry(item);
}

void AllItems::setRead(const QModelIndex& index) {
	if (!index.isValid())
		return;
	
	QModelIndex i = getItem(index);

	if (!i.isValid())
		return;

	TreeItem* item = const_cast<TreeItem*>(static_cast<const TreeItem*>(i.model()));
	item->setRead(i);
}

void AllItems::childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end) {
	row_count_ = -1; // Make it dirty
	
	qDebug() << __PRETTY_FUNCTION__;
	int rows = 0;
	FOREACH_CHILD
		if (item == sender)
			break;

		rows += item->rowCount(QModelIndex());
	}

	beginInsertRows(QModelIndex(), rows + start, rows + end);
	endInsertRows();
}

QString AllItems::real_id(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	if (item.isValid()) {
		return static_cast<const TreeItem*>(item.model())->real_id(item);
	} else {
		return id_;
	}
}

QString AllItems::content(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->content(item);
}

void AllItems::setStarred(const QModelIndex& index, bool starred) {
	QModelIndex item = getItem(index);

	if (item.isValid())
		const_cast<TreeItem*>(static_cast<const TreeItem*>(item.model()))->setStarred(item, starred);
}

void AllItems::setXpath(const QModelIndex& index, const QString& xpath) {
	QModelIndex item = getItem(index);

	if (item.isValid())
		const_cast<TreeItem*>(static_cast<const TreeItem*>(item.model()))->setXpath(item, xpath);
}

const QString& AllItems::xpath(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	if (item.isValid())
		static_cast<const TreeItem*>(item.model())->xpath(item);
}
