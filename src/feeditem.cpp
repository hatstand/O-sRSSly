#include "feeditem.h"
#include "readerapi.h"
#include "database.h"

#include <QSqlQuery>

QIcon FeedItem::sIcon;

FeedItem::FeedItem(TreeItem* parent, shared_ptr<FeedItemData> data)
	: TreeItem(parent, data->subscription().title()),
	  data_(data)
{
	id_ = data->subscription().id();
	connect(data.get(), SIGNAL(rowsInserted(int, int)), SLOT(feedRowsInserted(int, int)));
	
	if (sIcon.isNull())
		sIcon = QIcon(":feed.png");
}

QVariant FeedItem::data(int column, int role) const {
	switch (column) {
	case 1:
		return data_->subscription().id();
	default:
		return TreeItem::data(column, role);
	}
}

FeedItemData::FeedItemData(const Subscription& s, ReaderApi* api)
	: subscription_(s),
	  api_(api),
	  rowid_(-1)
{
	save();
	connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

FeedItemData::FeedItemData(const QSqlQuery& query, ReaderApi* api)
	: subscription_(query),
	  feed_(query),
	  api_(api),
	  rowid_(query.value(0).toLongLong())
{
	connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

FeedItemData::~FeedItemData() {
}

void FeedItemData::update() {
	api_->getSubscription(subscription_, feed_.continuation());
}

int FeedItemData::update(const AtomFeed& feed) {
	if (feed.id() == subscription_.id()) {
		qDebug() << "Update arrived for..." << subscription_.id() << feed.continuation();
		
		int beforeCount = feed_.entries().size();
		feed_.merge(feed);
		int afterCount = feed_.entries().size();

		if (beforeCount != afterCount) {
			emit rowsInserted(beforeCount, afterCount-1);
			QSqlDatabase::database().transaction();
			save();
			QSqlDatabase::database().commit();
			return afterCount - beforeCount;
		}
	}
	
	return 0;
}

int FeedItemData::update(const AtomEntry& e) {
	if (e.source == subscription_.id()) {
		qDebug() << "Single update arrived for..." << subscription_.id();
		int beforeCount = feed_.entries().size();
		feed_.add(e);
		int afterCount = feed_.entries().size();

		if (beforeCount != afterCount) {
			emit rowsInserted(beforeCount, afterCount-1);
			save();
			return 1;
		}
	}
	return 0;
}

void FeedItemData::setRead(const AtomEntry& e) {
	feed_.setRead(e);
	api_->setRead(e);
}

void FeedItemData::setStarred(const AtomEntry& e, bool starred) {
	feed_.setStarred(e, starred);
	api_->setStarred(e, starred);
	e.update();
}

void FeedItemData::setXpath(const QString& xpath) {
	subscription_.setXpath(xpath);
}

const QString& FeedItemData::xpath() const {
	return subscription_.xpath();
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
		if (!query.exec())
			Database::handleError(query.lastError());
		
		rowid_ = query.lastInsertId().toLongLong();
	} else {
		query.prepare("UPDATE Feed SET id=:id, title=:title, sortId=:sortId WHERE ROWID=:rowid");
		query.bindValue(":id", subscription_.id());
		query.bindValue(":title", subscription_.title());
		query.bindValue(":sortId", subscription_.sortid());
		query.bindValue(":rowid", rowid_);
		if (!query.exec())
			Database::handleError(query.lastError());
		
		query.prepare("DELETE FROM FeedTagMap WHERE feedId=:id");
		query.bindValue(":id", rowid_);
		if (!query.exec())
			Database::handleError(query.lastError());
	}
	
	// Save the list of tags it has
	query.prepare("INSERT INTO FeedTagMap (feedId, tagId) VALUES (:feedId, :tagId)");
	foreach (const Category& category, subscription_.categories()) {
		query.bindValue(":feedId", rowid_);
		query.bindValue(":tagId", category.first);
		if (!query.exec())
			Database::handleError(query.lastError());
	}
	
	// Save its entries
	feed_.saveEntries(rowid_);
}

QVariant FeedItem::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();
	
	const AtomEntry& e = data_->entries().at(index.row());

	switch (index.column()) {
		case Column_Title:
			return e.title;
		case Column_Read:
			return e.read;
		case Column_Date:
			return e.date;
		case Column_Preview:
			return e.previewText();
		case Column_Starred:
			return e.starred;
		case Column_Link:
			return e.link;
		case Column_Source:
			return e.source;
		case Column_Author:
			return e.author;
		case Column_SharedBy:
			return e.shared_by;
		case Column_Summary:
			return e.summary;
		case Column_Content:
			return e.content;
		case Column_Id:
			return id_;
		case Column_UnreadCount:
			return unread_count_;
		case Column_FeedName:
			return data_->subscription().title();
		case Column_Xpath:
			return data_->subscription().xpath();
		case Column_EntryId:
			return e.id;
		default:
			return QVariant();
	}
}

bool FeedItem::setData(const QModelIndex& index, const QVariant& value, int role) {
	const AtomEntry& e = data_->entries().at(index.row());
	switch (index.column()) {
		case Column_Read: {
			if (e.read)
				return true;

			data_->setRead(e);
			// Shhhhh... Don't tell anyone. We might disappear.
			/*QModelIndex top_left = createIndex(index.row(), 1);
			emit dataChanged(top_left, top_left);*/
			decrementUnreadCount();
			return true;
		}
		case Column_Starred:
			data_->setStarred(e, value.toBool());
			return true;

		case Column_Xpath:
			data_->setXpath(value.toString());
			return true;

		default:
			return false;
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

void FeedItem::feedRowsInserted(int from, int to) {
	beginInsertRows(QModelIndex(), from, to);
	endInsertRows();
	invalidateUnreadCount();
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

QString FeedItem::content(const QModelIndex& index) const {
	const AtomEntry& e = data_->entries().at(index.row());
	return e.content;
}

void FeedItem::setStarred(const QModelIndex& index, bool starred) {
	const AtomEntry& e = data_->entries().at(index.row());
	data_->setStarred(e, starred);
	e.update();

	QModelIndex top_left = createIndex(index.row(), 4);
	emit dataChanged(top_left, top_left);
}

