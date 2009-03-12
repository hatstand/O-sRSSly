#ifndef SHARED_ITEM_H
#define SHARED_ITEM_H

#include "folderitem.h"
#include "subscriptionlist.h"

class SharedItem : public FolderItem {
	Q_OBJECT
public:
	SharedItem(TreeItem* parent, ReaderApi* api, Database* db);
	virtual ~SharedItem() {}

	virtual void fetchMore(const QModelIndex& parent);

	static const QString kName;
	static const QString kId;
	static const Category kCategory;
};

#endif
