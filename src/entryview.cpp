#include "entryview.h"
#include "feeditem.h"
#include "treeitem.h"
#include "xmlutils.h"

#include <QAbstractProxyModel>
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTextDocument>

const int EntryDelegate::kMargin = 5;
const int EntryDelegate::kPreviewSpacing = 12;

EntryDelegate::EntryDelegate(QObject* parent)
	: QAbstractItemDelegate(parent),
	  headingMetrics_(QApplication::font()),
	  unreadMetrics_(QApplication::font()),
	  star_(":star.png")
{
	unreadFont_.setBold(true);
	headingMetrics_ = QFontMetrics(headingFont_);
	unreadMetrics_ = QFontMetrics(unreadFont_);
	
	itemHeight_ = qMax(20, QFontMetrics(headingFont_).height());
}

void EntryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	QString title(index.sibling(index.row(), TreeItem::Column_Title).data().toString());
	bool read(index.sibling(index.row(), TreeItem::Column_Read).data().toBool());
	QString preview(index.sibling(index.row(), TreeItem::Column_Preview).data().toString());
	bool starred(index.sibling(index.row(), TreeItem::Column_Starred).data().toBool());
	bool shared(index.sibling(index.row(), TreeItem::Column_Shared).data().toBool());
	
	XmlUtils::stripTags(title);
	
	QRect rect(option.rect);
	QColor headingColor(Qt::black);
	QColor previewColor(Qt::gray);
	QColor sharedColor(Qt::blue);
	
	// Draw selection background
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(rect, qApp->palette().color(QPalette::Highlight));
		headingColor = qApp->palette().color(QPalette::HighlightedText);
		//previewColor = Qt::
	}
	
	// Draw star
	rect.setLeft(kMargin);
	QRect starRect(rect.left(), rect.top() + (itemHeight_ - 16) / 2, 16, 16);
	painter->drawPixmap(starRect, star_.pixmap(starRect.size(), starred ? QIcon::Normal : QIcon::Disabled));
	
	// Draw heading
	rect.setLeft(starRect.right() + kMargin);
	painter->setPen(shared ? sharedColor : headingColor);
	painter->setFont(read ? headingFont_ : unreadFont_);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
	
	// Draw preview
	rect.setLeft(rect.left() + (read ? headingMetrics_ : unreadMetrics_).width(title) + kPreviewSpacing);
	if (rect.width() < 10)
		return;
	
	painter->setPen(previewColor);
	painter->setFont(previewFont_);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, preview);
}

QSize EntryDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	return QSize(100, itemHeight_);
}

bool EntryDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
	const QStyleOptionViewItem& option, const QModelIndex& index) {

	QModelIndex real_index = index.sibling(index.row(), TreeItem::Column_Starred); 
	bool starred = real_index.data().toBool();

	if (event->type() == QEvent::MouseButtonDblClick) {
		model->setData(real_index, !starred);

		return true;
	}

	return QAbstractItemDelegate::editorEvent(event, model, option, index);
}


EntryView::EntryView(QWidget* parent)
	: QListView(parent),
	  delegate_(new EntryDelegate(this))
{
	setItemDelegate(delegate_);
}

void EntryView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	if (selected.indexes().count() > 0)
		emit activated(selected.indexes()[0]);
	
	emitUpDown();
}

void EntryView::rowsInserted(const QModelIndex& parent, int start, int end) {
	QListView::rowsInserted(parent, start, end);
	
	emitUpDown();
}

void EntryView::emitUpDown() {
	emit canGoUpChanged(canGoUp());
	emit canGoDownChanged(canGoDown());
}

void EntryView::next() {
	moveSelection(+1);
}

void EntryView::previous() {
	moveSelection(-1);
}

void EntryView::moveSelection(int delta) {
	if ((delta < 0 && !canGoUp()) ||
	    (delta > 0 && !canGoDown()))
		return;
	
	QModelIndexList selection(selectionModel()->selectedIndexes());
	QModelIndex newSelection;
	
	// Figure out which item we've got to select
	if (selection.count() == 0)
		newSelection = model()->index(0, 0);
	else
		newSelection = selection[0].sibling(selection[0].row() + delta, 0);
	
	// Set our new selection
	selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
	
	// Repaint both the old selected item and the new selected item
	setDirtyRegion(visualRect(newSelection));
	if (selection.count() != 0)
		setDirtyRegion(visualRect(selection[0]));
	
	// Scroll to the new one
	scrollTo(newSelection);
}

bool EntryView::canGoUp() const {
	if (!selectionModel())
		return false;
	if (model()->rowCount() == 0)
		return false;
	
	QModelIndexList selection(selectionModel()->selectedIndexes());
	if (selection.count() == 0)
		return true;
	if (selection[0].row() <= 0) // Are we already at the start?
		return false;
	return true;
}

bool EntryView::canGoDown() const {
	if (!selectionModel())
		return false;
	if (model()->rowCount() == 0)
		return false;
	
	QModelIndexList selection(selectionModel()->selectedIndexes());
	if (selection.count() == 0)
		return true;
	if (selection[0].row() >= model()->rowCount(selection[0].parent()) - 1) // Are we already at the start?
		return false;
	return true;
}

