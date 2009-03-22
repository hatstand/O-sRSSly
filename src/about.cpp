#include "about.h"

#include <QMovie>

AboutBox::AboutBox(QWidget* parent)
	: QDialog(parent),
	  shoop_da_woop_(new QMovie(":/shoopdawoop.gif", QByteArray(), this))
{
	ui_.setupUi(this);
	ui_.shoop_->setMovie(shoop_da_woop_);
	shoop_da_woop_->start();
}

AboutBox::~AboutBox() {
}
