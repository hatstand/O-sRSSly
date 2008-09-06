#include "webclipview.h"

#include <QDebug>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QWebFrame>

QString WebclipView::kHelperJs;

WebclipView::WebclipView(QWidget* parent)
	: QWebView(parent),
	  webclipping_(false) {

	if (kHelperJs.isEmpty()) {
		QFile helper_js(":helpers.js");
		helper_js.open(QIODevice::ReadOnly);
		kHelperJs = helper_js.readAll();

		QFile webclip_js(":webclip.js");
		webclip_js.open(QIODevice::ReadOnly);
		kHelperJs += webclip_js.readAll();
	}

	connect(this, SIGNAL(loadFinished(bool)), SLOT(loaded()));
}

void WebclipView::mouseMoveEvent(QMouseEvent* event) {
	if (webclipping_) {
		QString js = "WebclipGetRect(document.elementFromPoint(%1, %2));";
		js = js.arg(event->x()).arg(event->y());

		QVariant ret = page()->currentFrame()->evaluateJavaScript(js);

		QString rect_string = ret.toString();
		if (rect_string.isEmpty())
			return;

		QStringList list = rect_string.split(',');
		int left = list[0].toInt();
		int top = list[1].toInt();
		int width = list[2].toInt();
		int height = list[3].toInt();

		current_rect_ = QRect(left, top - height, width, height);

		update();
	}

	QWebView::mouseMoveEvent(event);
}

void WebclipView::mousePressEvent(QMouseEvent* event) {
	if (webclipping_) {
		webclipping_ = false;
		QString js = "WebclipGetXpath(document.elementFromPoint(%1, %2));";
		js = js.arg(event->x()).arg(event->y());
		QVariant ret = page()->currentFrame()->evaluateJavaScript(js);
		qDebug() << ret;
		emit xpathSet(ret.toString());
	} else {
		QWebView::mousePressEvent(event);
	}
}

void WebclipView::paintEvent(QPaintEvent* event) {
	QWebView::paintEvent(event);

	int x_offset = page()->currentFrame()->scrollBarValue(Qt::Horizontal);
	int y_offset = page()->currentFrame()->scrollBarValue(Qt::Vertical);

	QRect draw_rect = current_rect_.translated(-x_offset, -y_offset);

	if (webclipping_ && current_rect_.isValid() && event->region().contains(draw_rect)) {
		QPainter p(this);
		
		draw_rect = draw_rect.intersected(page()->currentFrame()->geometry());

		p.setPen(Qt::blue);

		p.drawRect(draw_rect);
	}
}

void WebclipView::loaded() {
	// Insert our javascript into the page.
	page()->mainFrame()->evaluateJavaScript(kHelperJs);

	// See if we can webclip :-)
	if (!xpath_.isNull()) {
		qDebug() << "Webclipping...";
		QString js = "WebclipGetElement(\"%1\");";
		js = js.arg(xpath_);

		QString rect = page()->mainFrame()->evaluateJavaScript(js).toString();
		qDebug() << rect;

		QStringList list = rect.split(',');
		int left = list[0].toInt();
		int top = list[1].toInt();
		int width = list[2].toInt();
		int height = list[3].toInt();

		QRect snapshot(left, top - height, width, height);
		QPixmap image = QPixmap::grabWidget(this, snapshot);
		image.save("snapshot.png");
	}
}

void WebclipView::setUrl(const QUrl& url) {
	xpath_ = QString::null;
	QWebView::setUrl(url);
}

void WebclipView::getXpath(const QString& xpath) {
	qDebug() << __PRETTY_FUNCTION__ << xpath;
	xpath_ = xpath;
}
