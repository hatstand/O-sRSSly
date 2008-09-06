#ifndef FOLDERITEM_H
#define FOLDERITEM_H

#include "treeitem.h"

class ReaderApi;

/*
 * Represents a single folder/tag containing other feeds/folders.
 */
class FolderItem : public TreeItem {
	Q_OBJECT
public:
	FolderItem(TreeItem* parent, const QString& id, const QString& name, ReaderApi* api);
	FolderItem(TreeItem* parent, const QSqlQuery& query, ReaderApi* api);

	QString real_id(const QModelIndex& index) const;

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
	QString content(const QModelIndex& index) const;

	const AtomEntry& entry(const QModelIndex& index) const;
	void setRead(const QModelIndex& index);
	
	void setStarred(const QModelIndex& index, bool starred);

	void save();

	void setContinuation(const QString& continuation) { continuation_ = continuation; }

public slots:
	//virtual void childChanged(TreeItem* sender, const QModelIndex& top_left, const QModelIndex& bottom_right);

private:
	// Given an index for this item, returns an index for the correct child depending on the row.
	QModelIndex getItem(const QModelIndex& index) const;

	ReaderApi* api_;
	QString continuation_;
};

#endif
