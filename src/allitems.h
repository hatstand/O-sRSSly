#ifndef ALLITEMS_H
#define ALLITEMS_H

#include "treeitem.h"

/*
 * Folder that contains all other feeds
 */
class AllItems : public TreeItem {
	Q_OBJECT
public:
	AllItems(TreeItem* parent);
	
	int columnCount() const;

	// QAbstractTableModel
	// Returns the appropriate data from its children.
	// ie. a row index of 0 will return the first row from the first child.
	// A higher row index will return data from the appropriate row in the first child
	// or continue down through the children until enough rows have been counted.
	virtual QVariant data(const QModelIndex& index, int role) const;
	// The total number of rows from all the children.
	virtual int rowCount(const QModelIndex& parent) const;
	virtual bool canFetchMore(const QModelIndex& parent) const { return true; }
	virtual void fetchMore(const QModelIndex& parent);

	virtual QString summary(const QModelIndex& index) const;

	const AtomEntry& entry(const QModelIndex& index) const;
	void setRead(const QModelIndex& index);

public slots:
	virtual void childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end);

private:
	// Given an index for this item, returns an index for the correct child depending on the row.
	QModelIndex getItem(const QModelIndex& index) const;
	
	int row_count_;
};

#endif
