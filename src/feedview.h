#ifndef FEEDVIEW_H
#define FEEDVIEW_H

#include <QModelIndex>
#include <QTreeView>

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

private:
	QMenu* feed_menu_;

	QModelIndex clicked_index_;
};

#endif
