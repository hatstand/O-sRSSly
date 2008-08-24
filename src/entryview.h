#ifndef ENTRYVIEW_H
#define ENTRYVIEW_H

#include <QListView>
#include <QAbstractItemDelegate>


class EntryDelegate : public QAbstractItemDelegate {
public:
	EntryDelegate(QObject* parent = 0);
	
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	int itemHeight_;
	QFont headingFont_;
	QFontMetrics headingMetrics_;
	QFont previewFont_;
	
	static const int kMargin;
	static const int kPreviewSpacing;
};


class EntryView : public QListView {
	Q_OBJECT
public:
	EntryView(QWidget* parent = 0);
	virtual ~EntryView() {}

protected slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
	EntryDelegate* delegate_;
};

#endif
