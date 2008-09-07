#ifndef ALLITEMS_H
#define ALLITEMS_H

#include "treeitem.h"

class ReaderApi;
class FeedItem;

/*
 * Folder that contains all other feeds
 */
class AllItems : public TreeItem {
	Q_OBJECT
public:
	AllItems(TreeItem* parent, ReaderApi* api);
	
	// QAbstractTableModel
	// Returns the appropriate data from its children.
	// ie. a row index of 0 will return the first row from the first child.
	// A higher row index will return data from the appropriate row in the first child
	// or continue down through the children until enough rows have been counted.
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
	virtual QIcon icon() const { return sIcon; }
	// The total number of rows from all the children.
	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;
	virtual bool canFetchMore(const QModelIndex& parent) const { return true; }
	virtual void fetchMore(const QModelIndex& parent);

	const AtomEntry& entry(const QModelIndex& index) const;

public slots:
	virtual void childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end);
	void invalidateFeedCache();
	void regenerateFeedCache();

private:
	// Given an index for this item, returns an index for the correct child depending on the row.
	QModelIndex getItem(const QModelIndex& index) const;
	
	int row_count_;
	bool unique_feeds_dirty_;
	QList<FeedItem*> unique_feeds_;
	
	ReaderApi* api_;
	
	static QIcon sIcon;
};

#endif
