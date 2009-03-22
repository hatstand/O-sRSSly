#ifndef ABOUT_H
#define ABOUT_H

#include "ui_about.h"

#include <QDialog>

class ShoopDaWoop;

class AboutBox : public QDialog {
	Q_OBJECT
public:
	AboutBox(QWidget* parent);
	~AboutBox();

private slots:
	void enterFullscreen();
	void exitFullscreen();

private:
	Ui::About ui_;
	QMovie* shoop_da_woop_;
	ShoopDaWoop* fullscreen_;
};

#endif
