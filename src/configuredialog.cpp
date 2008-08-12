#include "configuredialog.h"

ConfigureDialog::ConfigureDialog(QWidget* parent)
	:	QDialog(parent) {
	setupUi(this);
}

ConfigureDialog::~ConfigureDialog() {

}

void ConfigureDialog::accept() {
	QString username(user_->text());
	QString password(password_->text());

	QDialog::accept();
}

void ConfigureDialog::show() {
	QDialog::show();
}
