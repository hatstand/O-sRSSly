#ifndef ABOUT_H
#define ABOUT_H

#include "ui_about.h"

#include <QDialog>

class QShowEvent;
class QHideEvent;
class ShoopDaWoop;

class AboutBox : public QDialog {
	Q_OBJECT
public:
	AboutBox(QWidget* parent);
	~AboutBox();

protected:
	void showEvent(QShowEvent*);
	void hideEvent(QHideEvent*);

private slots:
	void enterFullscreen();

private:
	Ui::About ui_;
	QMovie* shoop_da_woop_;
	ShoopDaWoop* fullscreen_;
};

#endif
