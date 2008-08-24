#include "entryview.h"
#include "treeitem.h"

#include <QPainter>
#include <QApplication>

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
	
	itemHeight_ = qMax(32, QFontMetrics(headingFont_).height() + kMargin*2);
}

void EntryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	QString title(index.sibling(index.row(), 0).data().toString());
	bool read(index.sibling(index.row(), 1).data().toBool());
	QString preview(index.sibling(index.row(), 3).data().toString());
	bool starred(index.sibling(index.row(), 4).data().toBool());
	
	QRect rect(option.rect);
	QColor headingColor(Qt::black);
	QColor previewColor(Qt::gray);
	
	// Draw selection background
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(rect, qApp->palette().color(QPalette::Highlight));
		headingColor = qApp->palette().color(QPalette::HighlightedText);
		//previewColor = Qt::
	}
	
	// Draw line between items
	painter->setPen(Qt::gray);
	painter->drawLine(rect.bottomLeft(), rect.bottomRight());
	
	// Draw star
	rect.setLeft(rect.left() + kMargin);
	rect.setTop(rect.top() + kMargin);
	rect.setBottom(rect.bottom() - kMargin);
	
	QRect starRect(rect.left(), rect.top(), 22, rect.height());
	painter->drawPixmap(starRect, star_.pixmap(22, 22, starred ? QIcon::Normal : QIcon::Disabled));
	
	// Draw heading
	rect.setLeft(starRect.right() + kPreviewSpacing);
	
	painter->setPen(headingColor);
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



EntryView::EntryView(QWidget* parent)
	: QListView(parent),
	  delegate_(new EntryDelegate(this))
{
	setItemDelegate(delegate_);
}

void EntryView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (selected.indexes().count() > 0)
		emit activated(selected.indexes()[0]);
}

