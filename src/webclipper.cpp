#include "webclipper.h"

#include <QDebug>
#include <QPainter>
#include <QWebFrame>

Webclipper::Webclipper(const QUrl& url, QObject* parent)
	: QThread(parent),
	  webpage_(NULL),
	  url_(url) {
}

void Webclipper::run() {
	qDebug() << __PRETTY_FUNCTION__;
	webpage_.reset(new QWebPage());

	webpage_->mainFrame()->load(url_);
	connect(webpage_.get(), SIGNAL(loadFinished(bool)), this, SLOT(loaded(bool)), Qt::DirectConnection);

	int ret = exec();

	if (ret != 0) {
		qDebug() << ":-( Event loop ended badly";
	}
}

void Webclipper::loaded(bool ok) {
	qDebug() << __PRETTY_FUNCTION__ << ok;

	if (ok) {
		webpage_->setViewportSize(webpage_->mainFrame()->contentsSize());
		QImage image(webpage_->viewportSize(), QImage::Format_ARGB32);
		QPainter p(&image);

		webpage_->mainFrame()->render(&p);

		image.save("w00t.png");
	} else {
		qDebug() << "Not webclipped successfully";
		exit(-1);
	}

	exit(0);
}
