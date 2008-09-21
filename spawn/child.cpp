#include "child.h"
#include "manager.h"
#include "spawnevent.pb.h"

#include <QDebug>
#include <QSharedMemory>
#include <QCoreApplication>
#include <QPainter>

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

void Child::sendResizeEvent(int width, int height) {
	resizeSharedMemory(width, height);
	
	ResizeEvent resizeEvent;
	resizeEvent.set_width(width);
	resizeEvent.set_height(height);
	*(resizeEvent.mutable_memory_key()) = memory_->key().toStdString();
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_RESIZE_EVENT);
	*(e.mutable_resize_event()) = resizeEvent;
	
	manager_->sendMessage(this, e);
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
	
	image = QImage(reinterpret_cast<uchar*>(memory_->data()), width, height, QImage::Format_RGB32);
}

void Child::clearQueue() {
	message_queue_.close();
	message_queue_.setBuffer(0);
	message_queue_.open(QBuffer::ReadWrite);
}

void Child::paint(QPainter& p) {
	memory_->lock();
	
	p.drawImage(0, 0, image);
	
	memory_->unlock();
}

}
