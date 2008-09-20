#include "view.h"
#include "manager.h"
#include "inputevents.pb.h"
#include "child.h"

#include <QEvent>
#include <QtDebug>

namespace Spawn {

View::View(Manager* manager, QWidget* parent)
	: QWidget(parent),
	  manager_(manager)
{
	child_ = manager_->createPage();
}

View::~View() {
	qDebug() << __PRETTY_FUNCTION__;
	manager_->destroyPage(child_);
}

bool View::event(QEvent* event) {
	switch (event->type()) {
	case QEvent::Resize: {
		ResizeEvent e;
		e.set_width(width());
		e.set_height(height());
		child_->sendResizeEvent(e);
		break;
	}
	default:
		break;
	}
	return QWidget::event(event);
}

}
