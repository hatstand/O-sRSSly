#include "allitems.h"
#include "feeditem.h"
#include "readerapi.h"

#include <QDebug>

#define FOREACH_CHILD \
	if (unique_feeds_dirty_) \
		const_cast<AllItems*>(this)->regenerateFeedCache(); \
	foreach (FeedItem* feed, unique_feeds_) {

QIcon AllItems::sIcon;

AllItems::AllItems(TreeItem* parent, ReaderApi* api)
	: TreeItem(parent, "All Items"),
	  row_count_(-1),
	  unique_feeds_dirty_(true),
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
	if (index.column() == Column_Preview && role == Qt::DisplayRole) {
		QModelIndex get_title = item.model()->index(item.row(), TreeItem::Column_FeedName, item.parent());
		return item.model()->data(get_title, role);
	}
	
	return item.model()->data(item, role);
}

bool AllItems::setData(const QModelIndex& index, const QVariant& value, int role) {
	QModelIndex item = getItem(index);

	if (!item.isValid())
		return false;
	
	return const_cast<QAbstractItemModel*>(item.model())->setData(item, value, role);
}

int AllItems::rowCount(const QModelIndex& parent) const {
	if (row_count_ == -1)
	{
		int& rowCountRef = const_cast<AllItems*>(this)->row_count_;
		rowCountRef = 0;
		
		FOREACH_CHILD
			rowCountRef += feed->rowCount(QModelIndex());
		}
	}
	return row_count_;
}

void AllItems::fetchMore(const QModelIndex& parent) {
	api_->getFresh();
}

QModelIndex AllItems::getItem(const QModelIndex& index) const {
	int rows = 0;
	
	FOREACH_CHILD
		int row_count = feed->rowCount(QModelIndex());
		rows += row_count;

		if (rows <= index.row())
			continue;
		
		int row_index = index.row() - rows + row_count;
		
		int safe_column = index.column();
		if (safe_column >= feed->columnCount(QModelIndex()))
			safe_column = feed->columnCount(QModelIndex()) - 1;
	
		return feed->index(row_index, safe_column);
	}

	qWarning() << "Reached end";
	return QModelIndex();
}

const AtomEntry& AllItems::entry(const QModelIndex& index) const {
	QModelIndex item = getItem(index);

	return static_cast<const TreeItem*>(item.model())->entry(item);
}

void AllItems::childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end) {
	row_count_ = -1; // Make it dirty
	
	int rows = 0;
	FOREACH_CHILD
		if (feed == sender)
			break;

		rows += feed->rowCount(QModelIndex());
	}

	beginInsertRows(QModelIndex(), rows + start, rows + end);
	endInsertRows();
}

void AllItems::invalidateFeedCache() {
	unique_feeds_dirty_ = true;
}

void AllItems::regenerateFeedCache() {
	unique_feeds_.clear();
	
	QSet<QString> dups;
	foreach (TreeItem* item, parent_->allChildren()) {
		FeedItem* feed = qobject_cast<FeedItem*>(item);
		if (!feed)
			continue;
		if (dups.contains(feed->subscription().id()))
			continue;
		dups << feed->subscription().id();
		unique_feeds_ << feed;
	}
	
	unique_feeds_dirty_ = false;
}

