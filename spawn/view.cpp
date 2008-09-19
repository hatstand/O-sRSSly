#include "view.h"
#include "manager.h"

namespace Spawn {

View::View(Manager* manager, QWidget* parent)
	: QWidget(parent),
	  manager_(manager)
{
	child_ = manager_->createPage();
}

View::~View() {
	manager_->destroyPage(child_);
}

bool View::event(QEvent* event) {
	return QWidget::event(event);
}

}
