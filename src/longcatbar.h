#ifndef LONGCATBAR_H
#define LONGCATBAR_H

#include <QProgressBar>
#include <QPixmap>

class QPaintEvent;

class LongCatBar : public QProgressBar {
	Q_OBJECT
public:
	LongCatBar(QWidget* parent = NULL);

protected:
	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);

private:
	static QPixmap* sHead;
	static QPixmap* sTail;
	static QPixmap* sMiddle;
	
	static const int kTailLength;
	static const int kHeadLength;
	
	static const Qt::TransformationMode kTransformationMode;
	
	QPixmap head_scaled_;
	QPixmap tail_scaled_;
	QPixmap middle_scaled_;
	
	float tail_scaled_length_;
	float head_scaled_length_;
};

#endif
