#include "page.h"
#include "qthelpers.h"
#include "spawnreply.pb.h"
#include "image.pb.h"

#include <QWebPage>
#include <QRect>
#include <QPainter>
#include <QWebFrame>
#include <QtDebug>
#include <QMouseEvent>
#include <QUrl>

namespace Spawn {

Page::Page(quint64 id)
	: QObject(),
	  id_(id),
	  page_(new QWebPage(this)),
	  no_recursion_please_(false)
{
	connect(page_, SIGNAL(repaintRequested(const QRect&)), SLOT(repaintRequested(const QRect&)));
	connect(page_, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
	connect(page_, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
	connect(page_, SIGNAL(loadStarted()), SLOT(loadStarted()));
	connect(page_, SIGNAL(statusBarMessage(const QString&)), SLOT(statusBarMessage(const QString&)));
	connect(page_, SIGNAL(linkClicked(const QUrl&)), SLOT(linkClicked(const QUrl&)));
	connect(page_, SIGNAL(scrollRequested(int, int, const QRect&)), SLOT(scrollRequested(int, int, const QRect&)));
	
	connect(page_->mainFrame(), SIGNAL(titleChanged(const QString&)), SLOT(titleChanged(const QString&)));
	connect(page_->mainFrame(), SIGNAL(urlChanged(const QUrl&)), SLOT(urlChanged(const QUrl&)));
}

Page::~Page() {
	qDebug() << __PRETTY_FUNCTION__;
}

void Page::resize(int width, int height) {
	image_ = QImage(width, height, QImage::Format_RGB32);
	page_->setViewportSize(QSize(width, height));
	repaintRequested();
}

void Page::repaintRequested(const QRect& region) {
	// Calls to QWebFrame::render sometimes seem to emit repaintRequested, which
	// calls this function again.  no_recursion_please_ prevents us double-locking
	// the shared memory.
	if (image_.isNull() || no_recursion_please_) {
		return;
	}
	//qDebug() << __PRETTY_FUNCTION__;
	
	// Lock the shared memory
	no_recursion_please_ = true;
	//memory_->lock();
	
	// Render the page
	QPainter p(&image_);
	if (region.isNull()) {
		page_->mainFrame()->render(&p);
	} else {
		page_->mainFrame()->render(&p, region);
	}
	p.end();
	
	// Unlock the shared memory
	//memory_->unlock();
	no_recursion_please_ = false;

	// Now construct a message to the GUI process to tell it
	// to redraw
	RepaintRequested r;
	if (region.isValid()) {
		r.set_x(region.x());
		r.set_y(region.y());
		r.set_w(region.width());
		r.set_h(region.height());
	}

	Image& i = *(r.mutable_image());
	i.set_w(image_.width());
	i.set_h(image_.height());
	i.set_data(image_.bits(), image_.numBytes());
	
	SpawnReply m;
	m.set_type(SpawnReply_Type_REPAINT_REQUESTED);
	m.set_source(id_);
	*(m.mutable_repaint_requested()) = r;
	
	emit reply(m);
}

void Page::mouseEvent(SpawnEvent_Type type, const MouseEvent& e) {
	QEvent::Type qtype;
	switch (type) {
	case SpawnEvent_Type_MOUSE_MOVE_EVENT:    qtype = QEvent::MouseMove;          break;
	case SpawnEvent_Type_MOUSE_PRESS_EVENT:   qtype = QEvent::MouseButtonPress;   break;
	case SpawnEvent_Type_MOUSE_RELEASE_EVENT: qtype = QEvent::MouseButtonRelease; break;
	default:
		return;
	}
	
	QPoint pos(e.x(), e.y());
	QMouseEvent event(qtype, pos,
		static_cast<Qt::MouseButton>(e.button()),
		Qt::MouseButtons(QFlag(e.buttons())),
		Qt::KeyboardModifiers(QFlag(e.modifiers()))
	);
	
	page_->event(&event);
}

void Page::wheelEvent(const MouseEvent& e) {
	QPoint pos(e.x(), e.y());
	QWheelEvent event(pos, e.delta(),
		Qt::MouseButtons(QFlag(e.buttons())),
		Qt::KeyboardModifiers(QFlag(e.modifiers())),
		static_cast<Qt::Orientation>(e.orientation())
	);
	
	page_->event(&event);
}

void Page::keyEvent(SpawnEvent_Type type, const KeyEvent& e) {
	QEvent::Type qtype;
	switch (type) {
	case SpawnEvent_Type_KEY_PRESS_EVENT:   qtype = QEvent::KeyPress;   break;
	case SpawnEvent_Type_KEY_RELEASE_EVENT: qtype = QEvent::KeyRelease; break;
	default:
		return;
	}
	
	QKeyEvent event(qtype, e.key(),
		Qt::KeyboardModifiers(QFlag(e.modifiers())),
		QString::fromStdString(e.text()),
		e.auto_repeat(),
		e.count()
	);
	
	page_->event(&event);
}

void Page::setUrl(const QUrl& url) {
	page_->mainFrame()->setUrl(url);
}

void Page::setLinkDelegationPolicy(int policy) {
	page_->setLinkDelegationPolicy(static_cast<QWebPage::LinkDelegationPolicy>(policy));
}

void Page::setHtml(const QString& html) {
	page_->mainFrame()->setHtml(html);
}

void Page::loadFinished(bool ok) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_LOAD_FINISHED);
	m.set_source(id_);
	m.set_simple_bool(ok);
	
	emit reply(m);
}

void Page::loadProgress(int progress) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_LOAD_PROGRESS);
	m.set_source(id_);
	m.set_simple_int(progress);
	
	emit reply(m);
}

void Page::loadStarted() {
	SpawnReply m;
	m.set_type(SpawnReply_Type_LOAD_STARTED);
	m.set_source(id_);
	
	emit reply(m);
}

void Page::statusBarMessage(const QString& text) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_STATUS_BAR_MESSAGE);
	m.set_source(id_);
	m.set_simple_string(text.toStdString());
	
	emit reply(m);
}

void Page::titleChanged(const QString& title) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_TITLE_CHANGED);
	m.set_source(id_);
	m.set_simple_string(title.toStdString());
	
	emit reply(m);
}

void Page::urlChanged(const QUrl& url) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_URL_CHANGED);
	m.set_source(id_);
	m.set_simple_string(url.toString().toStdString());
	
	emit reply(m);
}

void Page::linkClicked(const QUrl& url) {
	SpawnReply m;
	m.set_type(SpawnReply_Type_LINK_CLICKED);
	m.set_source(id_);
	m.set_simple_string(url.toString().toStdString());
	
	emit reply(m);
}

void Page::scrollRequested(int dx, int dy, const QRect& rectToScroll) {
	if (image_.isNull() || no_recursion_please_) {
		return;
	}
	
	// Lock the shared memory
	no_recursion_please_ = true;
	
	// Open a painter on the image
	QPainter p(&image_);
	p.setClipRect(image_.rect());
	
	// Copy the bit that has been scrolled
	QImage copy(image_.copy(rectToScroll));
	p.drawImage(rectToScroll.translated(dx, dy), copy, rectToScroll);
	
	// Render the new parts of the page that have been exposed
	QRect exposedX(QRegion(rectToScroll).subtracted(rectToScroll.translated(dx, 0)).boundingRect());
	QRect exposedY(QRegion(rectToScroll).subtracted(rectToScroll.translated(0, dy)).subtracted(QRegion(exposedX)).boundingRect());
	
	if (exposedX.isValid()) {
		page_->mainFrame()->render(&p, exposedX);
	}
	if (exposedY.isValid()) {
		page_->mainFrame()->render(&p, exposedY);
	}
	
	p.end();
	
	// Unlock the shared memory
	no_recursion_please_ = false;
	
	// Now tell the GUI to do the same
	ScrollRequested s;
	s.set_dx(dx);
	s.set_dy(dy);
	s.set_x(rectToScroll.x());
	s.set_y(rectToScroll.y());
	s.set_w(rectToScroll.width());
	s.set_h(rectToScroll.height());
	
	SpawnReply m;
	m.set_type(SpawnReply_Type_SCROLL_REQUESTED);
	m.set_source(id_);
	*(m.mutable_scroll_requested()) = s;
	
	emit reply(m);
}

}

