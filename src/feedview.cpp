#include "feedsmodel.h"
#include "feedview.h"
#include "settings.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QSignalMapper>
#include <QFontMetrics>
#include <QPainter>
#include <QApplication>

FeedDelegate::FeedDelegate(QObject* parent)
	: QAbstractItemDelegate(parent),
	  kIconSpacing(4)
{
	QFontMetrics metrics(normal_font_);
	item_height_ = qMax(18, metrics.height());
	
	unread_font_.setBold(true);
}

void FeedDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	QString title(index.sibling(index.row(), TreeItem::Column_Title).data().toString());
	QIcon icon(index.sibling(index.row(), TreeItem::Column_Title).data(Qt::DecorationRole).value<QIcon>());
	int unreadCount = index.sibling(index.row(), TreeItem::Column_UnreadCount).data().toInt();
	bool hasUnread = unreadCount > 0;
	
	if (hasUnread)
		title += " (" + QString::number(unreadCount) + ")";
	
	QRect rect(option.rect);
	QColor titleColor(Qt::black);
	QFont titleFont(hasUnread ? unread_font_ : normal_font_);
	
	// Draw selection background
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(option.rect, qApp->palette().color(QPalette::Highlight));
		titleColor = qApp->palette().color(QPalette::HighlightedText);
	}
	
	// Draw icon
	if (!icon.isNull()) {
		QRect iconRect(rect.left(), rect.top() + (item_height_ - 16) / 2, 16, 16);
		painter->drawPixmap(iconRect, icon.pixmap(iconRect.size()));
		
		rect.setLeft(iconRect.right() + kIconSpacing);
	}
	
	// Draw title
	painter->setPen(titleColor);
	painter->setFont(titleFont);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
}

QSize FeedDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(100, item_height_);
}


FeedView::FeedView(QWidget* parent)
	: QTreeView(parent),
	  feed_menu_(new QMenu(this)),
	  behaviour_menu_(new QMenu(this)),
	  delegate_(new FeedDelegate(this))
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
	behaviour_actions_[3] = new QAction("Webclip", behaviourGroup);
	behaviour_menu_->addActions(behaviourGroup->actions());
	
	QSignalMapper* mapper = new QSignalMapper(this);
	for (int i=0 ; i < behaviourGroup->actions().size(); ++i) {
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
	setItemDelegate(delegate_);
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

void FeedView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	Q_UNUSED(deselected);
	if (selected.indexes().count() > 0)
		emit activated(selected.indexes()[0]);
}
