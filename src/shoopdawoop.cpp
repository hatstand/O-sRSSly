#include "shoopdawoop.h"

#include <QMovie>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>

ShoopDaWoop::ShoopDaWoop()
	: QWidget(NULL),
	  movie_(new QMovie(":/shoopdawoop.gif"))
{
	setWindowTitle("SHOOP DA WOOP");
	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowStaysOnTopHint);
	connect(movie_, SIGNAL(updated(const QRect&)), SLOT(update()));
}

void ShoopDaWoop::showEvent(QShowEvent*) {
	setWindowState(Qt::WindowFullScreen);
	movie_->start();
}

void ShoopDaWoop::hideEvent(QHideEvent*) {
	movie_->stop();
	emit hidden();
}

void ShoopDaWoop::mousePressEvent(QMouseEvent* e) {
	hide();
	e->accept();
}

void ShoopDaWoop::keyPressEvent(QKeyEvent* e) {
	hide();
	e->accept();
}

void ShoopDaWoop::paintEvent(QPaintEvent*) {
	QPixmap frame(movie_->currentPixmap());
	frame = frame.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QRect rect(frame.rect());
	rect.translate((width() - rect.width())/2, (height() - rect.height())/2);

	QPainter p(this);
	p.drawPixmap(rect.topLeft(), frame);
}
