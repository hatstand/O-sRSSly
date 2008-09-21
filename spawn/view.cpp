#include "view.h"
#include "manager.h"
#include "inputevents.pb.h"
#include "child.h"

#include <QEvent>
#include <QtDebug>
#include <QPainter>
#include <QPaintEvent>

namespace Spawn {

View::View(Manager* manager, QWidget* parent)
	: QWidget(parent),
	  manager_(manager)
{
	child_ = manager_->createPage();
	connect(child_, SIGNAL(repaintRequested(const QRect&)), SLOT(repaintRequested(const QRect&)));
	connect(child_, SIGNAL(loadFinished(bool)), SIGNAL(loadFinished(bool)));
	connect(child_, SIGNAL(loadProgress(int)), SIGNAL(loadProgress(int)));
	connect(child_, SIGNAL(loadStarted()), SIGNAL(loadStarted()));
	connect(child_, SIGNAL(statusBarMessage(const QString&)), SIGNAL(statusBarMessage(const QString&)));
	connect(child_, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
	connect(child_, SIGNAL(urlChanged(const QUrl&)), SIGNAL(urlChanged(const QUrl&)));
	
	setMouseTracking(true);
}

View::~View() {
	qDebug() << __PRETTY_FUNCTION__;
	manager_->destroyPage(child_);
}

void View::resizeEvent(QResizeEvent*) {
	child_->sendResizeEvent(width(), height());
}

void View::mouseMoveEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_MOVE_EVENT, e);
}

void View::mousePressEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_PRESS_EVENT, e);
}

void View::mouseReleaseEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_RELEASE_EVENT, e);
}

void View::keyPressEvent(QKeyEvent* e) {
	child_->sendKeyEvent(SpawnEvent_Type_KEY_PRESS_EVENT, e);
}

void View::keyReleaseEvent(QKeyEvent* e) {
	child_->sendKeyEvent(SpawnEvent_Type_KEY_RELEASE_EVENT, e);
}

void View::wheelEvent(QWheelEvent* e) {
	child_->sendWheelEvent(e);
}

void View::paintEvent(QPaintEvent* e) {
	QPainter p(this);
	p.setClipRect(e->rect());
	
	switch (child_->state()) {
	case Child::Ready:
		child_->paint(p, e->rect());
		break;
	
	case Child::Error:
		drawMessagePage(p, "Error!", QColor(157, 0, 0));
		break;
	
	case Child::Starting:
		drawMessagePage(p, "Starting...", QColor(251, 255, 171));
		break;
	}
}

void View::repaintRequested(const QRect& rect) {
	if (rect.isNull())
		update();
	else
		update(rect);
}

void View::drawMessagePage(QPainter& p, const QString& msg, const QColor& backgroundColor) {
	p.fillRect(rect(), backgroundColor);
	QRect textRect(rect().adjusted(10, 10, -10, -10));
	
	QFont font;
	font.setPointSize(18);
	font.setBold(true);
	
	p.setFont(font);
	p.drawText(textRect, msg, Qt::AlignHCenter | Qt::AlignTop);
}


}
