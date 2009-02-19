#include "longcatbar.h"
#include "settings.h"

#include <QPaintEvent>
#include <QPainter>
#include <QtDebug>

QPixmap* LongCatBar::sHead = NULL;
QPixmap* LongCatBar::sTail = NULL;
QPixmap* LongCatBar::sMiddle = NULL;

const int LongCatBar::kTailLength = 140;
const int LongCatBar::kHeadLength = 112;

const Qt::TransformationMode LongCatBar::kTransformationMode = Qt::SmoothTransformation;

LongCatBar::LongCatBar(QWidget* parent)
	: QProgressBar(parent)
{
	if (sHead == NULL) {
		sHead = new QPixmap(":longcat-head.png");
		sTail = new QPixmap(":longcat-tail.png");
		sMiddle = new QPixmap(":longcat-middle.png");
	}
	
	connect(Settings::instance(), SIGNAL(progressBarStyleChanged(int)), SLOT(update()));
}

void LongCatBar::resizeEvent(QResizeEvent* event) {
	Q_UNUSED(event);
	head_scaled_ = sHead->scaledToHeight(height(), kTransformationMode);
	tail_scaled_ = sTail->scaledToHeight(height(), kTransformationMode);
	middle_scaled_ = sMiddle->scaledToHeight(height(), kTransformationMode);
	
	float scale = head_scaled_.height() / float(sHead->height());
	
	head_scaled_length_ = scale * kHeadLength;
	tail_scaled_length_ = scale * kTailLength;
}

void LongCatBar::paintEvent(QPaintEvent* e) {
	if (Settings::instance()->progressBarStyle() == Settings::ProgressBar_Normal) {
		QProgressBar::paintEvent(e);
		return;
	}
	
	QPainter p(this);
	
	int barWidth = width() - int(head_scaled_length_ - tail_scaled_length_);
	barWidth *= int(float(value() - minimum()) / maximum());
	
	// Keep drawing middle bits until we've filled up the entire bar width
	for (int widthLeft = barWidth, x = int(tail_scaled_length_) ; widthLeft > 0 ; widthLeft -= middle_scaled_.width()) {
		int widthToDraw = qMin(widthLeft, middle_scaled_.width());
		p.drawPixmap(QRect(x, 0, widthToDraw, height()), middle_scaled_, QRect(0, 0, widthToDraw, height()));
		x += widthToDraw;
	}
	
	// Draw the tail and head
	p.drawPixmap(0, 0, tail_scaled_);
	p.drawPixmap(int(tail_scaled_length_ + barWidth - (head_scaled_.width() - head_scaled_length_)), 0, head_scaled_);
}
