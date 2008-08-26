#ifndef WEBCLIPVIEW_H
#define WEBCLIPVIEW_H

#include <QWebView>

class WebclipView : public QWebView {
	Q_OBJECT
public:
	WebclipView(QWidget* parent = 0);
	void setWebclipping(bool enable = true) { webclipping_ = enable; }
private:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void paintEvent(QPaintEvent* event);

private slots:
	void loaded();

private:
	bool webclipping_;
	QRect current_rect_;

	static const QString kGetRectJs;
	static QString kHelperJs;
};

#endif
