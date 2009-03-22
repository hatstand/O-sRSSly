#include "about.h"
#include "shoopdawoop.h"

#include <QMovie>
#include <QLabel>

AboutBox::AboutBox(QWidget* parent)
	: QDialog(parent),
	  shoop_da_woop_(new QMovie(":/shoopdawoop.gif", QByteArray(), this)),
	  fullscreen_(NULL)
{
	ui_.setupUi(this);
	ui_.shoop_->setMovie(shoop_da_woop_);
	shoop_da_woop_->start();

	connect(ui_.fullscreen_, SIGNAL(clicked()), SLOT(enterFullscreen()));
}

AboutBox::~AboutBox() {
	delete fullscreen_;
}

void AboutBox::enterFullscreen() {
	if (!fullscreen_) {
		fullscreen_ = new ShoopDaWoop();
		connect(fullscreen_, SIGNAL(hidden()), SLOT(exitFullscreen()));
	}

	shoop_da_woop_->stop();
	fullscreen_->show();
}

void AboutBox::exitFullscreen() {
	shoop_da_woop_->start();
}

