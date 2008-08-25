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
	QFont unreadFont_;
	QFontMetrics headingMetrics_;
	QFontMetrics unreadMetrics_;
	QFont previewFont_;
	
	QIcon star_;
	
	static const int kMargin;
	static const int kPreviewSpacing;
};


class EntryView : public QListView {
	Q_OBJECT
public:
	EntryView(QWidget* parent = 0);
	virtual ~EntryView() {}
	
	bool canGoUp() const;
	bool canGoDown() const;

signals:
	void canGoUpChanged(bool);
	void canGoDownChanged(bool);

public slots:
	void next();
	void previous();

protected slots:
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void rowsInserted(const QModelIndex& parent, int start, int end);

private slots:
	void emitUpDown();

private:
	void moveSelection(int delta);
	EntryDelegate* delegate_;
};

#endif
