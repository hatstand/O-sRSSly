#include "child.h"
#include "manager.h"
#include "spawnevent.pb.h"

#include <QDebug>

namespace Spawn {

Child::Child(Manager* manager, quint64 id)
	: QObject(manager),
	  manager_(manager),
	  id_(id),
	  ready_(false)
{
	qDebug() << __PRETTY_FUNCTION__;
}

void Child::setReady() {
	qDebug() << __PRETTY_FUNCTION__;
	ready_ = true;
	emit ready();
}

void Child::sendMouseEvent(const MouseEvent& mouseEvent) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_MOUSE_EVENT);
	*(e.mutable_mouse_event()) = mouseEvent;
	
	manager_->sendMessage(this, e);
}

}
