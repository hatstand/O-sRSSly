#ifndef TREEITEM_H
#define TREEITEM_H

#include "atomfeed.h"

#include <QAbstractTableModel>

/*
 * Represents a single node in the tree model.
 *
 * Also represents a list of Atom entries as a QAbstractTableModel
 */
class TreeItem : public QAbstractTableModel {
	Q_OBJECT
public:
	TreeItem(TreeItem* parent = 0, const QString& title = QString());
	virtual ~TreeItem();
	
	void clear();

	// TreeModel stuff.
	void appendChild(TreeItem* child);
	TreeItem* child(int row);
	int childCount() const;
	virtual int columnCount() const = 0;
	// Which child this item is.
	int row() const;
	// Get some actual data. Default implementation returns just the title.
	virtual QVariant data(int column) const;
	TreeItem* parent();

	QString title() const;

	virtual QString id() const { return id_; }

	// Get the atom summary for the given index.
	virtual QString summary(const QModelIndex& index) const = 0;
	// Gets the appropriate AtomEntry.
	virtual const AtomEntry& entry(const QModelIndex& index) const = 0;
	// Sets the AtomEntry to read.
	virtual void setRead(const QModelIndex& index) = 0;

	// QAbstractTableModel
	virtual int columnCount(const QModelIndex& parent) const;
	// Returns Atom data, such as title, date and read status.
	virtual QVariant data(const QModelIndex& index, int role) const = 0;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	// Number of atom entries within/below this node.
	virtual int rowCount(const QModelIndex& parent) const = 0;
	
	virtual void save() {}

public slots:
	// Called when a child item is reset.
	virtual void childReset();
	// Called when a child's data changes. Indices are from the child.
	virtual void childChanged(const QModelIndex& top_left, const QModelIndex& bottom_right);
	// Called when a child is destroyed.
	virtual void childDestroyed(QObject* object);


protected:
	TreeItem* parent_;
	QList<TreeItem*> children_;
	QString title_;
	QString id_;
	qint64 rowid_;
};

#endif
