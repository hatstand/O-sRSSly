#ifndef ROOTITEM_H
#define ROOTITEM_H

#include "treeitem.h"

class RootItem : public TreeItem {
	Q_OBJECT
public:
	RootItem(TreeItem* parent) : TreeItem(parent, "/") {}
	
	int columnCount() const { return 1; }
	
	virtual QString summary(const QModelIndex& index) const { return QString::null; }
	QString real_id(const QModelIndex& index) const { return QString::null; }
	QString content(const QModelIndex& index) const { return QString::null; }

	const AtomEntry& entry(const QModelIndex& index) const { return sDummy; }
	void setRead(const QModelIndex& index) {}

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex& index, int role) const { return QVariant(); }
	virtual int rowCount(const QModelIndex& parent) const { return 0; }

private:
	static AtomEntry sDummy;
};

#endif
