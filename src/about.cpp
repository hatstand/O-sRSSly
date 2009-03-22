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

	connect(ui_.fullscreen_, SIGNAL(clicked()), SLOT(enterFullscreen()));
}

AboutBox::~AboutBox() {
	delete fullscreen_;
}

void AboutBox::enterFullscreen() {
	if (!fullscreen_) {
		fullscreen_ = new ShoopDaWoop();
	}

	fullscreen_->show();
	hide();
}

void AboutBox::showEvent(QShowEvent*) {
	shoop_da_woop_->start();
}

void AboutBox::hideEvent(QHideEvent*) {
	shoop_da_woop_->stop();
}
