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
}

View::~View() {
	qDebug() << __PRETTY_FUNCTION__;
	manager_->destroyPage(child_);
}

bool View::event(QEvent* event) {
	switch (event->type()) {
	case QEvent::Resize: {
		child_->sendResizeEvent(width(), height());
		break;
	}
	default:
		break;
	}
	return QWidget::event(event);
}

void View::paintEvent(QPaintEvent* event) {
	qDebug() << __PRETTY_FUNCTION__;
	QPainter p(this);
	child_->paint(p);
}

void View::repaintRequested(const QRect& rect) {
	update(rect);
}

}
