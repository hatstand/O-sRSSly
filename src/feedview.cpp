#include "feedsmodel.h"
#include "feedview.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>

FeedView::FeedView(QWidget* parent)
	: QTreeView(parent), feed_menu_(new QMenu(this)) {
	QAction* update = new QAction("Update", feed_menu_);
	feed_menu_->addAction(update);
	QAction* rename_feed = new QAction("Rename", feed_menu_);
	feed_menu_->addAction(rename_feed);

	connect(update, SIGNAL(triggered()), SLOT(updateClicked()));
	connect(rename_feed, SIGNAL(triggered()), SLOT(renameFeedClicked()));

	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::InternalMove);
	setDropIndicatorShown(true);
}

FeedView::~FeedView() {
}

void FeedView::contextMenuEvent(QContextMenuEvent* event) {
	clicked_index_ = indexAt(event->pos());
	if (!clicked_index_.isValid())
		return;

	QPoint global_pos = mapToGlobal(event->pos());
	feed_menu_->popup(global_pos);
}

void FeedView::updateClicked() {
	if (!clicked_index_.isValid())
		return;

	QAbstractItemModel* m = static_cast<const FeedsModel*>(model())->getEntries(clicked_index_);
	if (m && m->canFetchMore(QModelIndex()))
		m->fetchMore(QModelIndex());
}

void FeedView::renameFeedClicked() {
	qDebug() << __PRETTY_FUNCTION__;
}

