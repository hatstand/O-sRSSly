#include "child.h"
#include "manager.h"

#include "spawnreply.pb.h"

#include <QDebug>
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
	  state_(Starting)
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
	if (width <= 0 || height <= 0) {
		return;
	}

	pixmap_ = QPixmap(width, height);
	
	ResizeEvent resizeEvent;
	resizeEvent.set_width(width);
	resizeEvent.set_height(height);
	
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_RESIZE_EVENT);
	*(e.mutable_resize_event()) = resizeEvent;
	
	manager_->sendMessage(this, e);
}

void Child::clearQueue() {
	message_queue_.close();
	message_queue_.setBuffer(0);
	message_queue_.open(QBuffer::ReadWrite);
}

void Child::paint(QPainter& p, const QRect& rect) {
	if (state_ != Ready)
		return;
	
	p.drawPixmap(rect, pixmap_, rect);
}

void Child::setUrl(const QUrl& url) {
	SpawnEvent e;
	e.set_destination(id_);
	e.set_type(SpawnEvent_Type_SET_URL);
	*(e.mutable_simple_string()) = url.toString().toStdString();
	
	manager_->sendMessage(this, e);
	
	setLastUrl(url);
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
	
	setLastHtml(html);
}

void Child::setLastUrl(const QUrl& url) {
	last_url_ = url;
	last_html_ = QString::null;
}

void Child::setLastHtml(const QString& html) {
	last_url_ = QUrl();
	last_html_ = html;
}

void Child::restoreState() {
	if (!last_html_.isNull()) {
		setHtml(last_html_);
	} else {
		setUrl(last_url_);
	}
}

void Child::repaintRequested(const Image& image, const QRect& rect) {
	QImage img(reinterpret_cast<const uchar*>(image.data().data()), image.w(), image.h(), QImage::Format_ARGB32);

	QPainter p(&pixmap_);
	p.drawImage(rect, img, rect);
	p.end();

	emit repaintRequested(rect);
}

}
