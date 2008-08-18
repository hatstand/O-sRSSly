#ifndef FOLDERITEM_H
#define FOLDERITEM_H

#include "treeitem.h"

class FolderItem : public TreeItem {
	Q_OBJECT
public:
	FolderItem(TreeItem* parent, const QString& id, const QString& name);
	int columnCount() const;
	TreeItem::Type rtti() const { return TreeItem::Folder; }

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

	virtual QString summary(const QModelIndex& index) const;

	const AtomEntry& entry(const QModelIndex& index) const;

private:
	QModelIndex getItem(const QModelIndex& index) const;
	QString id_;
};

#endif
