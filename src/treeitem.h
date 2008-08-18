#ifndef TREEITEM_H
#define TREEITEM_H

#include "atomfeed.h"

#include <QAbstractTableModel>

class TreeItem : public QAbstractTableModel {
	Q_OBJECT
public:
	enum Type {
		Feed = 0,
		Folder
	};

	TreeItem(TreeItem* parent = 0, const QString& title = QString());

	void appendChild(TreeItem* child);
	TreeItem* child(int row);
	int childCount() const;
	virtual int columnCount() const = 0;
	int row() const;
	virtual QVariant data(int column) const;
	virtual Type rtti() const = 0;

	TreeItem* parent();

	QString title() const;

	virtual QString summary(const QModelIndex& index) const = 0;

	virtual const AtomEntry& entry(const QModelIndex& index) const = 0;

	// QAbstractTableModel
	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index, int role) const = 0;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual int rowCount(const QModelIndex& parent) const = 0;

protected:
	TreeItem* parent_;
	QList<TreeItem*> children_;
	QString title_;
};

#endif
