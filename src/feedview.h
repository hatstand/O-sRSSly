#ifndef FEEDVIEW_H
#define FEEDVIEW_H

#include <QModelIndex>
#include <QTreeView>
#include <QFont>


class FeedDelegate : public QAbstractItemDelegate {
public:
	FeedDelegate(QObject* parent = 0);
	
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	int item_height_;
	
	QFont unread_font_;
	QFont normal_font_;
};


class FeedView : public QTreeView {
	Q_OBJECT
public:
	FeedView(QWidget* parent = 0);
	virtual ~FeedView();

protected:
	void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void updateClicked();
	void renameFeedClicked();
	void behaviourChanged(int);

private:
	QMenu* feed_menu_;
	QMenu* behaviour_menu_;
	
	QAction* behaviour_actions_[3];

	QModelIndex clicked_index_;
	QString clicked_id_;
	
	FeedDelegate* delegate_;
};

#endif
