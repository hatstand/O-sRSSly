#include "feedsmodel.h"
#include "feedview.h"
#include "settings.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QSignalMapper>

FeedView::FeedView(QWidget* parent)
	: QTreeView(parent),
	  feed_menu_(new QMenu(this)),
	  behaviour_menu_(new QMenu(this))
{
	QAction* update = new QAction("Update", feed_menu_);
	feed_menu_->addAction(update);
	QAction* rename_feed = new QAction("Rename", feed_menu_);
	feed_menu_->addAction(rename_feed);
	feed_menu_->addSeparator();
	
	feed_menu_->addMenu(behaviour_menu_)->setText("Behaviour");
	
	QActionGroup* behaviourGroup = new QActionGroup(feed_menu_);
	behaviourGroup->setExclusive(true);
	behaviour_actions_[0] = new QAction("Decide automatically", behaviourGroup);
	behaviour_actions_[1] = new QAction("Show article summary", behaviourGroup);
	behaviour_actions_[2] = new QAction("Open linked webpage", behaviourGroup);
	behaviour_menu_->addActions(behaviourGroup->actions());
	
	QSignalMapper* mapper = new QSignalMapper(this);
	for (int i=0 ; i<3 ; ++i) {
		behaviour_actions_[i]->setCheckable(true);
		mapper->setMapping(behaviour_actions_[i], i);
		connect(behaviour_actions_[i], SIGNAL(triggered(bool)), mapper, SLOT(map()));
	}
	connect(mapper, SIGNAL(mapped(int)), SLOT(behaviourChanged(int)));

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
	
	clicked_id_ = clicked_index_.sibling(clicked_index_.row(), 1).data().toString();
	behaviour_menu_->setEnabled(!clicked_id_.isEmpty());
	behaviour_actions_[Settings::instance()->behaviour(clicked_id_)]->setChecked(true);

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

void FeedView::behaviourChanged(int behaviour) {
	Settings::instance()->setBehaviour(clicked_id_, behaviour);
}

