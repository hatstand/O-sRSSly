#include "webpage_process.h"

#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QWebFrame>

#include <cstring>

WebpageProcess::WebpageProcess(const QUrl& url, QObject* parent)
	: QObject(parent),
	  url_(url),
	  page_(new QWebPage(this)),
	  shared_memory_(url.toString(), this) {
	connect(page_.get(), SIGNAL(loadFinished(bool)), SLOT(pageLoaded(bool)));
	page_->mainFrame()->load(url);
}

void WebpageProcess::pageLoaded(bool ok) {
	qDebug() << __PRETTY_FUNCTION__;
	emit loaded(url_);
}

QSize WebpageProcess::render() {
	qDebug() << __PRETTY_FUNCTION__;
	QSize page_size = page_->mainFrame()->contentsSize();
	page_->setViewportSize(page_size);
	Q_ASSERT(page_->viewportSize() == page_size);

	int num_bytes = 4 * page_size.width() * page_size.height();

	if (!shared_memory_.isAttached()) {
		if (!shared_memory_.attach(QSharedMemory::ReadWrite)) {
			if (!shared_memory_.create(num_bytes, QSharedMemory::ReadWrite)) {
				qFatal("Could not create shared memory:" + shared_memory_.error());
			}
		}
	}

	if (shared_memory_.size() < num_bytes) {
		qFatal("Shared memory not large enough");
	}

	shared_memory_.lock();
	QImage image((uchar*)shared_memory_.data(), page_size.width(), page_size.height(), QImage::Format_ARGB32);
	QPainter painter(&image);
	page_->mainFrame()->render(&painter);
	painter.end();
	shared_memory_.unlock();

	return page_size;
}
