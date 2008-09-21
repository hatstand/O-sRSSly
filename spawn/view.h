#ifndef SPAWNVIEW_H
#define SPAWNVIEW_H

#include "child.h"

#include <QWidget>
#include <QUrl>

class QWebView;

namespace Spawn {

class Manager;

class View : public QWidget {
	Q_OBJECT
public:
	View(Manager* manager, QWidget* parent = 0);
	~View();
	
	void setUrl(const QUrl& url);

signals:
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void statusBarMessage(const QString& text);
	void titleChanged(const QString& title);
	void urlChanged(const QUrl& url);

protected:
	void resizeEvent(QResizeEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void wheelEvent(QWheelEvent* e);

private slots:
	void repaintRequested(const QRect& rect);
	void childStateChanged(Child::State state);
	void messageLinkClicked(const QUrl& url);

private:
	int page_id_;
	Manager* manager_;
	Child* child_;
	QWebView* message_view_;
};

}

#endif
