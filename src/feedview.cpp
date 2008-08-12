#include "feedsmodel.h"
#include "feedview.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>

FeedView::FeedView(QWidget* parent)
	: QTreeView(parent), feed_menu_(new QMenu(this)), folder_menu_(new QMenu(this)) {
	QAction* update = new QAction("Update", feed_menu_);
	feed_menu_->addAction(update);
	QAction* rename_folder = new QAction("Rename", folder_menu_);
	folder_menu_->addAction(rename_folder);
	QAction* rename_feed = new QAction("Rename", feed_menu_);
	feed_menu_->addAction(rename_feed);

	connect(update, SIGNAL(triggered()), SLOT(updateClicked()));
	connect(rename_folder, SIGNAL(triggered()), SLOT(renameFolderClicked()));
	connect(rename_feed, SIGNAL(triggered()), SLOT(renameFeedClicked()));
}

FeedView::~FeedView() {
}

void FeedView::contextMenuEvent(QContextMenuEvent* event) {
	clicked_index_ = indexAt(event->pos());
	if (!clicked_index_.isValid())
		return;

	TreeItem* item = static_cast<TreeItem*>(clicked_index_.internalPointer());

	QPoint global_pos = mapToGlobal(event->pos());

	if (item->rtti() == TreeItem::Feed) {
		feed_menu_->popup(global_pos);
	} else if (item->rtti() == TreeItem::Folder) {
		folder_menu_->popup(global_pos);
	}
}

void FeedView::updateClicked() {
	qDebug() << __PRETTY_FUNCTION__;

	if (clicked_index_.isValid() && model()->canFetchMore(clicked_index_)) {
		model()->fetchMore(clicked_index_);
	}
}

void FeedView::renameFeedClicked() {
	qDebug() << __PRETTY_FUNCTION__;
}

void FeedView::renameFolderClicked() {
	qDebug() << __PRETTY_FUNCTION__;
}
