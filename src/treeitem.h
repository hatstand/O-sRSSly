#ifndef TREEITEM_H
#define TREEITEM_H

#include "atomfeed.h"

#include <QAbstractTableModel>
#include <QIcon>

class FeedsModel;

/*
 * Represents a single node in the tree model.
 *
 * Also represents a list of Atom entries as a QAbstractTableModel
 */
class TreeItem : public QAbstractTableModel {
	Q_OBJECT
public:
	TreeItem(TreeItem* parent, const QString& title = QString());
	TreeItem(FeedsModel* model, const QString& title = QString());
	virtual ~TreeItem();
	
	void clear();

	// TreeModel stuff.
	void appendChild(TreeItem* child);
	TreeItem* child(int row);
	const QList<TreeItem*>& children() const { return children_; }
	QList<TreeItem*> allChildren() const;
	int childCount() const;
	// Which child this item is.
	int row() const;
	// Get some actual data. Default implementation returns just the title.
	virtual QVariant data(int column, int role = Qt::DisplayRole) const;
	TreeItem* parent();
	FeedsModel* feedsModel() const { return feeds_model_; }
	QModelIndex indexInFeedsModel() const;
	virtual QIcon icon() const { return QIcon(); }

	QString title() const;

	virtual QString id() const { return id_; }
	virtual QString real_id(const QModelIndex&) const = 0;
	virtual QString content(const QModelIndex&) const = 0;

	// Get the atom summary for the given index.
	virtual QString summary(const QModelIndex& index) const = 0;
	// Gets the appropriate AtomEntry.
	virtual const AtomEntry& entry(const QModelIndex& index) const = 0;
	// Sets the AtomEntry to read.
	virtual void setRead(const QModelIndex& index) = 0;
	int unreadCount() const;
	void invalidateUnreadCount();
	void decrementUnreadCount();

	virtual void setStarred(const QModelIndex& index, bool starred = true) = 0;

	virtual void setXpath(const QModelIndex& index, const QString& xpath) = 0;
	virtual const QString& xpath(const QModelIndex& index) const { return QString::null; }

	// QAbstractTableModel
	virtual int columnCount(const QModelIndex& parent) const;
	// Returns Atom data, such as title, date and read status.
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const = 0;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	// Number of atom entries within/below this node.
	virtual int rowCount(const QModelIndex& parent) const = 0;
	
	virtual void save() {}
	
	void installChangedProxy(TreeItem* other) { changed_proxy_ = other; }

signals:
	void unreadCountChanged(const QModelIndex& index);

public slots:
	// Called when rows are inserted into a child
	virtual void childRowsInserted(TreeItem* sender, const QModelIndex& parent, int start, int end);

private slots:
	// Called when a child item is reset.
	void childReset();
	// Called when rows are inserted into a child
	void childRowsInserted(const QModelIndex& parent, int start, int end);
	// Called when a child is destroyed.
	void childDestroyed(QObject* object);

private:
	void init(TreeItem* parent, const QString& title, FeedsModel* model);

protected:
	TreeItem* parent_;
	TreeItem* changed_proxy_;
	bool fail_prevention_;
	QList<TreeItem*> children_;
	QString title_;
	QString id_;
	qint64 rowid_;
	int unread_count_;
	
	FeedsModel* feeds_model_;
};

#endif
