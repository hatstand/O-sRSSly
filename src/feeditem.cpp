#include "feeditem.h"
#include "readerapi.h"


FeedItem::FeedItem(TreeItem* parent, FeedItemData* data)
	: TreeItem(parent, data->subscription().title()), data_(data) {
	id_ = data->subscription().id();
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
			return data_->subscription().id();
		default:
			return QVariant();
	}
}

void FeedItemData::update() {
	api_->getSubscription(subscription_, feed_.continuation());
	connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

void FeedItemData::update(const AtomFeed& feed) {
	if (feed.id() == subscription_.id()) {
		qDebug() << "Update arrived for..." << subscription_.id();
		disconnect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), this, 0);
		feed_.merge(feed);
		emit updated();
	}
}

void FeedItemData::setRead(const AtomEntry& e) {
	feed_.setRead(e);
	api_->setRead(e);
}

void FeedItemData::addCategory(const QPair<QString,QString>& category) {
	subscription_.addCategory(category);
	api_->addCategory(subscription_, category.first);
}

QVariant FeedItem::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	
	const AtomEntry& e = data_->entries().at(index.row());

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
	return data_->entries().size();
}

QString FeedItem::summary(const QModelIndex& index) const {
	const AtomEntry& e = data_->entries().at(index.row());
	return e.summary;
}

const AtomEntry& FeedItem::entry(const QModelIndex& index) const {
	return data_->entries().at(index.row());
}

void FeedItem::setRead(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	if (!index.isValid())
		return;

	const AtomEntry& e = data_->entries().at(index.row());
	if (e.read)
		return;
	
	data_->setRead(e);

	QModelIndex top_left = createIndex(index.row(), 1);
	emit dataChanged(top_left, top_left);
}

void FeedItem::feedUpdated() {
	reset();
}

void FeedItem::fetchMore(const QModelIndex&) {
	data_->update();
}

void FeedItem::addCategory(const QPair<QString,QString>& category) {
	data_->addCategory(category);
}
