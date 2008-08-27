#include "feeditem.h"
#include "readerapi.h"

#include <QSqlQuery>

FeedItem::FeedItem(TreeItem* parent, shared_ptr<FeedItemData> data)
	: TreeItem(parent, data->subscription().title()),
	  data_(data)
{
	id_ = data->subscription().id();
	connect(data.get(), SIGNAL(rowsInserted(int, int)), SLOT(feedRowsInserted(int, int)));
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

FeedItemData::FeedItemData(const Subscription& s, ReaderApi* api)
	: subscription_(s),
	  api_(api),
	  rowid_(-1)
{
	save();
}

FeedItemData::FeedItemData(const QSqlQuery& query, ReaderApi* api)
	: subscription_(query),
	  feed_(query),
	  api_(api),
	  rowid_(query.value(0).toLongLong())
{
}

FeedItemData::~FeedItemData() {
	qDebug() << __PRETTY_FUNCTION__;
}

void FeedItemData::update() {
	api_->getSubscription(subscription_, feed_.continuation());
	connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

void FeedItemData::update(const AtomFeed& feed) {
	if (feed.id() == subscription_.id()) {
		qDebug() << "Update arrived for..." << subscription_.id();
		disconnect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), this, 0);
		
		int beforeCount = feed_.entries().size();
		feed_.merge(feed);
		int afterCount = feed_.entries().size();
		
		if (beforeCount != afterCount) {
			emit rowsInserted(beforeCount, afterCount-1);
			save();
		}
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

void FeedItemData::removeCategory(const QString& category) {
	qDebug() << "Removing" << category << "from" << subscription_.id();
	subscription_.removeCategory(category);
	api_->removeCategory(subscription_, category);
}

void FeedItemData::save() {
	QSqlQuery query;
	
	// Save the feed itself
	if (rowid_ == -1) {
		query.prepare("INSERT INTO Feed (id, title, sortId) VALUES (:id, :title, :sortId)");
		query.bindValue(":id", subscription_.id());
		query.bindValue(":title", subscription_.title());
		query.bindValue(":sortId", subscription_.sortid());
		query.exec();
		
		rowid_ = query.lastInsertId().toLongLong();
	} else {
		query.prepare("UPDATE Feed SET id=:id, title=:title, sortId=:sortId WHERE ROWID=:rowid");
		query.bindValue(":id", subscription_.id());
		query.bindValue(":title", subscription_.title());
		query.bindValue(":sortId", subscription_.sortid());
		query.bindValue(":rowid", rowid_);
		query.exec();
		
		query.prepare("DELETE FROM FeedTagMap WHERE feedId=:id");
		query.bindValue(":id", rowid_);
		query.exec();
	}
	
	// Save the list of tags it has
	query.prepare("INSERT INTO FeedTagMap (feedId, tagId) VALUES (:feedId, :tagId)");
	foreach (const Category& category, subscription_.categories()) {
		query.bindValue(":feedId", rowid_);
		query.bindValue(":tagId", category.first);
		query.exec();
	}
	
	// Save its entries
	feed_.saveEntries(rowid_);
}

QVariant FeedItem::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	
	const AtomEntry& e = data_->entries().at(index.row());

	switch (index.column()) {
		case 0:
			return e.title;
		case 1:
			return e.read;
		case 2:
			return e.date;
		case 3:
			return e.previewText();
		case 4:
			return false; // TODO: Starred
		case 5:
			return e.link;
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

void FeedItem::feedRowsInserted(int from, int to) {
	qDebug() << __PRETTY_FUNCTION__;
	beginInsertRows(QModelIndex(), from, to);
	endInsertRows();
}

void FeedItem::fetchMore(const QModelIndex&) {
	data_->update();
}

void FeedItem::addCategory(const QPair<QString,QString>& category) {
	data_->addCategory(category);
}

QString FeedItem::real_id(const QModelIndex& index) const {
	return id_;
}
