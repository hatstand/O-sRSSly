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

FeedItemData::FeedItemData(Subscription* s, ReaderApi* api, Database* db)
	: subscription_(s),
	  feed_(s->id(), db),
	  api_(api),
	  db_(db)
{
	save();
	//connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

FeedItemData::FeedItemData(const QSqlQuery& query, ReaderApi* api, Database* db)
	: subscription_(new Subscription(query, db)),
	  feed_(query, db),
	  api_(api),
	  db_(db)
{
	//connect(api_, SIGNAL(subscriptionArrived(const AtomFeed&)), SLOT(update(const AtomFeed&)));
}

FeedItemData::~FeedItemData() {
}

void FeedItemData::update() {
	api_->getSubscription(*subscription_, feed_.continuation());
}

int FeedItemData::update(const AtomFeed& feed) {
	if (feed.id() == subscription_->id()) {
		qDebug() << "Update arrived for..." << subscription_->id() << feed.continuation();
		
		int beforeCount = feed_.entries().size();
		feed_.merge(feed);
		int afterCount = feed_.entries().size();

		if (beforeCount != afterCount) {
			emit rowsInserted(beforeCount, afterCount-1);
			save();
			return afterCount - beforeCount;
		}
	}
	
	return 0;
}

int FeedItemData::update(const AtomEntry& e) {
	if (e.source == subscription_->id()) {
		qDebug() << "Single update arrived for..." << subscription_->id();
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
	subscription_->setXpath(xpath);
}

const QString& FeedItemData::xpath() const {
	return subscription_->xpath();
}

void FeedItemData::addCategory(const QPair<QString,QString>& category, bool update_account) {
	subscription_->addCategory(category);
	if (update_account)
		api_->addCategory(*subscription_, category.first);
}

void FeedItemData::removeCategory(const QString& category) {
	qDebug() << "Removing" << category << "from" << subscription_->id();
	subscription_->removeCategory(category);
	api_->removeCategory(*subscription_, category);
}

void FeedItemData::save() {
	// Save the feed itself
	QList<QVariant> bind_values;
	bind_values << subscription_->id();
	bind_values << subscription_->title();
	bind_values << subscription_->sortid();
	db_->pushQuery(
		"REPLACE INTO Feed (id, title, sortId) VALUES (:id, :title, :sortId)",
		bind_values);
		
	// Clear out the old mappings
	bind_values.clear();
	bind_values << subscription_->id();
	db_->pushQuery("DELETE FROM FeedTagMap WHERE feedId=:id", bind_values);
	
	// Save the list of tags it has
	QString query("INSERT INTO FeedTagMap (feedId, tagId) VALUES (:feedId, :tagId)");
	foreach (const Category& category, subscription_->categories()) {
		bind_values.clear();
		bind_values << subscription_->id();
		bind_values << category.first;
		db_->pushQuery(query, bind_values);
	}
	
	// Save its entries
	feed_.saveEntries();
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

