#ifndef SHOOPDAWOOP_H
#define SHOOPDAWOOP_H

#include <QWidget>

class QMovie;
class QShowEvent;
class QHideEvent;
class QMouseEvent;
class QKeyEvent;
class QResizeEvent;
class QPaintEvent;

class ShoopDaWoop : public QWidget {
	Q_OBJECT
public:
	ShoopDaWoop();

signals:
	void hidden();

protected:
	void showEvent(QShowEvent* e);
	void hideEvent(QHideEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void paintEvent(QPaintEvent* e);

private:
	QMovie* movie_;
};


#endif

