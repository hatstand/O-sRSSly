#include "child.h"
#include "manager.h"

#include <QDebug>
#include <QSharedMemory>
#include <QCoreApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QUrl>

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
	emit stateChanged(state_);
}

void Child::setError() {
	qDebug() << __PRETTY_FUNCTION__;
	state_ = Error;
	
	// TODO: Clean this up properly, because the child process won't have
	delete memory_;
	memory_ = NULL;
	image_ = QImage();
	
	emit stateChanged(state_);
	emit repaintRequested(QRect());
}

void Child::sendMouseEvent(SpawnEvent_Type type, QMouseEvent* event) {
	if (!isReady()) {
		return;
	}
	
	MouseEvent mouseEvent;
	mouseEvent.set_x(event->pos().x());
	mouseEvent.set_y(event->pos().y());
	mouseEvent.set_button(event->button());
	mouseEvent.set_buttons(event->buttons());
	mouseEvent.set_modifiers(event->modifiers());
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(type);
	*(e.mutable_mouse_event()) = mouseEvent;
	
	manager_->sendMessage(this, e);
}

void Child::sendWheelEvent(QWheelEvent* event) {
	if (!isReady()) {
		return;
	}
	
	MouseEvent mouseEvent;
	mouseEvent.set_x(event->pos().x());
	mouseEvent.set_y(event->pos().y());
	mouseEvent.set_buttons(event->buttons());
	mouseEvent.set_modifiers(event->modifiers());
	mouseEvent.set_delta(event->delta());
	mouseEvent.set_orientation(event->orientation());
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_WHEEL_EVENT);
	*(e.mutable_mouse_event()) = mouseEvent;
	
	manager_->sendMessage(this, e);
}

void Child::sendKeyEvent(SpawnEvent_Type type, QKeyEvent* event) {
	if (!isReady()) {
		return;
	}
	
	KeyEvent keyEvent;
	keyEvent.set_key(event->key());
	keyEvent.set_modifiers(event->modifiers());
	keyEvent.set_text(event->text().toStdString());
	keyEvent.set_auto_repeat(event->isAutoRepeat());
	keyEvent.set_count(event->count());
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(type);
	*(e.mutable_key_event()) = keyEvent;
	
	manager_->sendMessage(this, e);
}

void Child::sendResizeEvent(int width, int height) {
	if (!resizeSharedMemory(width, height)) {
		return;
	}
	
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

bool Child::resizeSharedMemory(int width, int height) {
	int size = width*height*4;
	
	if (size <= 0) {
		return false;
	}
	
	delete memory_;
	memory_ = new QSharedMemory(this);
	
	while (true) {
		QString key = "feeder-" + QString::number(QCoreApplication::applicationPid()) + "-" + QString::number(qrand());
		memory_->setKey(key);
		if (memory_->create(size, QSharedMemory::ReadOnly)) {
			break;
		}
	}
	
	image_ = QImage(reinterpret_cast<uchar*>(memory_->data()), width, height, QImage::Format_RGB32);
	
	return true;
}

void Child::clearQueue() {
	message_queue_.close();
	message_queue_.setBuffer(0);
	message_queue_.open(QBuffer::ReadWrite);
}

void Child::paint(QPainter& p, const QRect& rect) {
	if (state_ != Ready || !memory_)
		return;
	
	memory_->lock();
	p.drawImage(rect, image_, rect);
	memory_->unlock();
}

void Child::setUrl(const QUrl& url) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_SET_URL);
	*(e.mutable_simple_string()) = url.toString().toStdString();
	
	manager_->sendMessage(this, e);
}

void Child::setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_SET_LINK_DELEGATION_POLICY);
	e.set_simple_int(policy);
	
	manager_->sendMessage(this, e);
}

void Child::setHtml(const QString& html) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_SET_HTML);
	e.set_simple_string(html.toStdString());
	
	manager_->sendMessage(this, e);
}

}
