#include "child.h"
#include "manager.h"
#include "spawnevent.pb.h"

#include <QDebug>
#include <QSharedMemory>
#include <QCoreApplication>

namespace Spawn {

Child::Child(Manager* manager, quint64 id)
	: QObject(manager),
	  manager_(manager),
	  id_(id),
	  state_(Starting),
	  memory_(NULL)
{
	qDebug() << __PRETTY_FUNCTION__;
	
	message_queue_.open(QBuffer::ReadWrite);
}

void Child::setReady() {
	qDebug() << __PRETTY_FUNCTION__;
	state_ = Ready;
	emit ready();
}

void Child::setError() {
	qDebug() << __PRETTY_FUNCTION__;
	state_ = Error;
	emit error();
	
	// TODO: Clean this up properly, because the child process won't have
	delete memory_;
	memory_ = NULL;
}

void Child::sendMouseEvent(const MouseEvent& mouseEvent) {
	if (!isReady()) {
		return;
	}
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_MOUSE_EVENT);
	*(e.mutable_mouse_event()) = mouseEvent;
	
	manager_->sendMessage(this, e);
}

void Child::sendResizeEvent(const ResizeEvent& resizeEvent) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_RESIZE_EVENT);
	*(e.mutable_resize_event()) = resizeEvent;
	
	manager_->sendMessage(this, e);
	
	resizeSharedMemory(resizeEvent.width(), resizeEvent.height());
}

void Child::resizeSharedMemory(int width, int height) {
	int size = width*height*4;
	
	delete memory_;
	memory_ = new QSharedMemory(this);
	
	while (true) {
		QString key = "feeder-" + QString::number(QCoreApplication::applicationPid()) + "-" + QString::number(qrand());
		memory_->setKey(key);
		if (memory_->create(size, QSharedMemory::ReadOnly)) {
			break;
		}
	}
	
	SharedMemoryChanged m;
	*(m.mutable_key()) = memory_->key().toStdString();
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_SHARED_MEMORY_CHANGED);
	*(e.mutable_shared_memory_changed()) = m;
	
	manager_->sendMessage(this, e);
}

void Child::clearQueue() {
	message_queue_.close();
	message_queue_.setBuffer(0);
	message_queue_.open(QBuffer::ReadWrite);
}

}
