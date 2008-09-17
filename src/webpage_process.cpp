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
	page_->setViewportSize(page_->mainFrame()->contentsSize());
	QImage image(page_->viewportSize(), QImage::Format_ARGB32);
	QPainter painter(&image);

	page_->mainFrame()->render(&painter);
	painter.end();

	shared_memory_.create(image.numBytes(), QSharedMemory::ReadWrite);
	shared_memory_.lock();
	memcpy(shared_memory_.data(), image.bits(), image.numBytes());
	shared_memory_.unlock();

	return image.size();
}
