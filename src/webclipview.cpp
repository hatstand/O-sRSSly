#include "webclipview.h"

#include <QDebug>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QWebFrame>

QString WebclipView::kHelperJs;

const QString WebclipView::kGetRectJs =
	"var element = document.elementFromPoint(%1, %2);"

	"var l = WebclipGetAbsoluteLeft(element);"
	"var t = WebclipGetAbsoluteTop(element) + element.offsetHeight;"
	"var rect = \"\";"
	"rect += l + \",\";"
	"rect += t + \",\";"
	"rect += element.offsetWidth + \",\";"
	"rect += element.offsetHeight;"
	"rect;";

WebclipView::WebclipView(QWidget* parent)
	: QWebView(parent),
	  webclipping_(false) {

	if (kHelperJs.isEmpty()) {
		QFile helper_js(":helpers.js");
		helper_js.open(QIODevice::ReadOnly);
		kHelperJs = helper_js.readAll();
	}

	connect(this, SIGNAL(loadFinished(bool)), SLOT(loaded()));
}

void WebclipView::mouseMoveEvent(QMouseEvent* event) {
	if (webclipping_) {
		QString js = kGetRectJs;
		js = kGetRectJs.arg(event->x()).arg(event->y());

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
}
