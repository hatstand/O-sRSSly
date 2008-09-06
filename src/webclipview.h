#ifndef WEBCLIPVIEW_H
#define WEBCLIPVIEW_H

#include <QWebView>

class WebclipView : public QWebView {
	Q_OBJECT
public:
	WebclipView(QWidget* parent = 0);
	void setWebclipping(bool enable = true) { webclipping_ = enable; }
	virtual void setUrl(const QUrl& url);
	void getXpath(const QString& xpath);
private:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void paintEvent(QPaintEvent* event);

signals:
	void xpathSet(const QString& xpath);

private slots:
	void loaded();

private:
	bool webclipping_;
	QRect current_rect_;

	QString xpath_;

	static QString kHelperJs;
};

#endif
