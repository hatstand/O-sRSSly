#include "view.h"
#include "manager.h"
#include "inputevents.pb.h"
#include "child.h"

#include <QEvent>
#include <QtDebug>
#include <QPainter>

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

void View::paintEvent(QPaintEvent*) {
	QPainter p(this);
	child_->paint(p);
}

void View::repaintRequested(const QRect& rect) {
	update(rect);
}

}
