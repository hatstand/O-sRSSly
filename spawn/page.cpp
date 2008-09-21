#include "page.h"
#include "spawnreply.pb.h"

#include <QWebPage>
#include <QRect>
#include <QSharedMemory>
#include <QPainter>
#include <QWebFrame>
#include <QtDebug>

namespace Spawn {

Page::Page(quint64 id)
	: QObject(),
	  id_(id),
	  page_(new QWebPage(this)),
	  memory_(NULL),
	  no_recursion_please_(false)
{
	connect(page_, SIGNAL(repaintRequested(const QRect&)), SLOT(repaintRequested(const QRect&)));
	page_->mainFrame()->load(QUrl("http://www.google.com"));
}

Page::~Page() {
}

void Page::resize(int width, int height, const QString& memoryKey) {
	delete memory_;
	memory_ = new QSharedMemory(memoryKey, this);
	if (memory_->attach()) {
		image_ = QImage(reinterpret_cast<uchar*>(memory_->data()), width, height, QImage::Format_RGB32);
		repaintRequested();
	} else {
		image_ = QImage();
	}
	
	page_->setViewportSize(QSize(width, height));
}

void Page::repaintRequested(const QRect& region) {
	// Calls to QWebFrame::render sometimes seem to emit repaintRequested, which
	// calls this function again.  no_recursion_please_ prevents us double-locking
	// the shared memory.
	if (image_.isNull() || no_recursion_please_) {
		return;
	}
	qDebug() << __PRETTY_FUNCTION__;
	
	no_recursion_please_ = true;
	memory_->lock();
	
	QPainter p(&image_);
	page_->mainFrame()->render(&p, region);
	p.end();
	
	memory_->unlock();
	no_recursion_please_ = false;
	
	RepaintRequested r;
	if (region.isValid()) {
		r.set_x(region.x());
		r.set_y(region.y());
		r.set_w(region.width());
		r.set_h(region.height());
	}
	
	SpawnReply m;
	m.set_type(SpawnReply_Type_REPAINT_REQUESTED);
	m.set_source(id_);
	*(m.mutable_repaint_requested()) = r;
	
	emit reply(m);
}

}

