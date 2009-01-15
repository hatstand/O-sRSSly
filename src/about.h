#ifndef ABOUT_H
#define ABOUT_H

#include "ui_about.h"

#include <QDialog>

class AboutBox : public QDialog {
	Q_OBJECT
public:
	AboutBox(QWidget* parent);
	~AboutBox();

private:
	Ui::About ui_;
};

#endif
