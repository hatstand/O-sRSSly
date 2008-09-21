#ifndef SPAWNVIEW_H
#define SPAWNVIEW_H

#include <QWidget>
#include <QUrl>

namespace Spawn {

class Manager;
class Child;

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

private:
	int page_id_;
	Manager* manager_;
	Child* child_;
};

}

#endif
