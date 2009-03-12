#include "shareditem.h"

#include "readerapi.h"

const QString SharedItem::kName = "Shared Items";
const QString SharedItem::kId = "OsrsslyShared";
const Category SharedItem::kCategory(kId, kName);

SharedItem::SharedItem(TreeItem* parent, ReaderApi* api, Database* db)
	: FolderItem(parent, kId, kName, api, db) {
}

void SharedItem::fetchMore(const QModelIndex&) {
	api_->getFriends();
	// TODO: Should get our shared items here too
}
