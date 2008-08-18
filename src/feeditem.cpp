#include "feeditem.h"

FeedItem::FeedItem(TreeItem* parent, FeedItemData* data)
	: TreeItem(parent, data->subscription_.title()), data_(data) {
	connect(data, SIGNAL(updated()), SLOT(feedUpdated()));
}

int FeedItem::columnCount() const {
	return 2;
}

QVariant FeedItem::data(int column) const {
	switch (column) {
		case 0:
			return title_;
		case 1:
			return data_->subscription_.id();
		default:
			return QVariant();
	}
}

void FeedItemData::FeedItemData::update(const AtomFeed& feed) {
	qDebug() << __PRETTY_FUNCTION__;
	feed_.merge(feed);

	emit updated();
}

QVariant FeedItem::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	
	const AtomEntry& e = data_->feed_.entries()[index.row()];

	switch (index.column()) {
		case 0:
			return e.title;
		case 1:
			return (e.read ? "Read" : "Unread");
		case 2:
			return e.date;
		default:
			return QVariant();
	}
}

int FeedItem::rowCount(const QModelIndex& parent) const {
	return data_->feed_.entries().size();
}

QString FeedItem::summary(const QModelIndex& index) const {
	const AtomEntry& e = data_->feed_.entries()[index.row()];
	return e.summary;
}

const AtomEntry& FeedItem::entry(const QModelIndex& index) const {
	return data_->feed_.entries()[index.row()];
}

void FeedItem::setRead(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	if (!index.isValid())
		return;

	const AtomEntry& e = data_->feed_.entries()[index.row()];
	if (e.read)
		return;
	
	data_->feed_.setRead(e);

	QModelIndex top_left = createIndex(index.row(), 1);
	emit dataChanged(top_left, top_left);
}

void FeedItem::feedUpdated() {
	reset();
}
