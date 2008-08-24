#include "entryview.h"
#include "treeitem.h"

#include <QPainter>
#include <QApplication>

const int EntryDelegate::kMargin = 5;
const int EntryDelegate::kPreviewSpacing = 12;

EntryDelegate::EntryDelegate(QObject* parent)
	: QAbstractItemDelegate(parent),
	  headingMetrics_(QApplication::font())
{
	//headingFont_.setBold(true);
	headingMetrics_ = QFontMetrics(headingFont_);
	
	itemHeight_ = qMax(32, QFontMetrics(headingFont_).height() + kMargin*2);
}

void EntryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	QModelIndex title(index.sibling(index.row(), 0));
	QModelIndex preview(index.sibling(index.row(), 3));
	
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
	
	// Draw heading
	QString headingText(title.data().toString());
	rect.setLeft(rect.left() + kMargin);
	rect.setTop(rect.top() + kMargin);
	rect.setBottom(rect.bottom() - kMargin);
	
	painter->setPen(headingColor);
	painter->setFont(headingFont_);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, headingText);
	
	// Draw preview
	QString previewText(preview.data().toString());
	rect.setLeft(headingMetrics_.width(headingText) + kMargin + kPreviewSpacing);
	if (rect.width() < 10)
		return;
	
	painter->setPen(previewColor);
	painter->setFont(previewFont_);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, previewText);
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

