#ifndef ROOTITEM_H
#define ROOTITEM_H

#include "treeitem.h"

class RootItem : public TreeItem {
	Q_OBJECT
public:
	RootItem(FeedsModel* model) : TreeItem(model, "/") {}
	
	virtual QString summary(const QModelIndex&) const { return QString::null; }
	QString real_id(const QModelIndex&) const { return QString::null; }
	QString content(const QModelIndex&) const { return QString::null; }

	const AtomEntry& entry(const QModelIndex&) const { return sDummy; }
	void setStarred(const QModelIndex&, bool) {}

	void setXpath(const QModelIndex&, const QString&) {}

	// QAbstractTableModel
	virtual QVariant data(const QModelIndex&, int) const { return QVariant(); }
	virtual int rowCount(const QModelIndex&) const { return 0; }
	virtual int columnCount(const QModelIndex&) const { return 0; }

private:
	static AtomEntry sDummy;
};

#endif
