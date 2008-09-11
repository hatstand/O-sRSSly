#include "webclipper.h"
#include "webclipview.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QReadLocker>
#include <QWebFrame>
#include <QWriteLocker>

Webclipper::Webclipper(const QUrl& url, const QString& xpath, QObject* parent)
	: QThread(parent),
	  url_(url),
	  xpath_(xpath) {
}

void Webclipper::run() {
	qDebug() << __PRETTY_FUNCTION__;
	QWriteLocker locker(&webpage_mutex_);
	webpage_.reset(new QWebPage());

	webpage_->mainFrame()->load(url_);
	connect(webpage_.get(), SIGNAL(loadFinished(bool)), this, SLOT(loaded(bool)), Qt::DirectConnection);

	int ret = exec();

	if (ret != 0) {
		qDebug() << ":-( Event loop ended badly";
	}

	// Move the QWebPage to the main thread.
	webpage_->moveToThread(QApplication::instance()->thread());
}

void Webclipper::loaded(bool ok) {
	qDebug() << __PRETTY_FUNCTION__ << ok;

	if (ok) {
		// Load helpers
		webpage_->mainFrame()->evaluateJavaScript(WebclipView::getHelperJs());
		webpage_->setViewportSize(webpage_->mainFrame()->contentsSize());
		QImage image(webpage_->viewportSize(), QImage::Format_ARGB32);

		// See if we can webclip :-)
		if (!xpath_.isNull()) {
			qDebug() << "Webclipping...";
			QString js = "WebclipGetElement(\"%1\");";
			js = js.arg(xpath_);

			QString rect = webpage_->mainFrame()->evaluateJavaScript(js).toString();
			qDebug() << rect;

			QStringList list = rect.split(',');
			int left = list[0].toInt();
			int top = list[1].toInt();
			int width = list[2].toInt();
			int height = list[3].toInt();

			QRect snapshot(left, top - height, width, height);
			QPainter p(&image);
			webpage_->mainFrame()->render(&p, snapshot);
			// QPainter must be finished before we try and use the target.
			p.end();
			image = image.copy(snapshot);
		} else {
			QPainter p(&image);
			webpage_->mainFrame()->render(&p);
		}

		QWriteLocker locker(&image_mutex_);
		snapshot_ = image;
	} else {
		qDebug() << "Not webclipped successfully";
		exit(-1);
	}

	exit(0);
}

QImage Webclipper::snapshot() {
	QReadLocker locker(&image_mutex_);
	return snapshot_;
}

shared_ptr<QWebPage> Webclipper::webpage() {
	QReadLocker locker(&webpage_mutex_);
	
	return webpage_;
}
