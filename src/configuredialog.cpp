#include "configuredialog.h"
#include "settings.h"

ConfigureDialog::ConfigureDialog(QWidget* parent)
	: QDialog(parent),
	  settings_(Settings::instance())
{
	ui_.setupUi(this);
	
	connect(ui_.pageList_, SIGNAL(currentTextChanged(const QString&)), SLOT(pageChanged(const QString&)));
	ui_.pageList_->setCurrentRow(0);
}

ConfigureDialog::~ConfigureDialog()
{
}

void ConfigureDialog::accept()
{
	settings_->setGoogleAccount(ui_.user_->text(), ui_.password_->text());

	QDialog::accept();
}

void ConfigureDialog::show()
{
	populateData();
	QDialog::show();
}

int ConfigureDialog::exec()
{
	populateData();
	return QDialog::exec();
}

void ConfigureDialog::populateData()
{
	ui_.user_->setText(settings_->googleUsername());
	ui_.password_->setText(settings_->googlePassword());
}

void ConfigureDialog::pageChanged(const QString& text)
{
	ui_.pageTitle_->setText("<b>" + text + "</b>");
}
