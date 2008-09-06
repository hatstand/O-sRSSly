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
	Settings::ProgressBarStyle progressBarStyle;
	if (ui_.progressBarNormal->isChecked())   progressBarStyle = Settings::ProgressBar_Normal;
	if (ui_.progressBarLongcat->isChecked())  progressBarStyle = Settings::ProgressBar_Longcat;
	if (ui_.progressBarTacgnol->isChecked())  progressBarStyle = Settings::ProgressBar_Tacgnol;
	
	settings_->setGoogleAccount(ui_.user_->text(), ui_.password_->text());
	settings_->setProgressBarStyle(progressBarStyle);

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
	
	switch (settings_->progressBarStyle()) {
		case Settings::ProgressBar_Normal:  ui_.progressBarNormal->setChecked(true);  break;
		case Settings::ProgressBar_Longcat: ui_.progressBarLongcat->setChecked(true); break;
		case Settings::ProgressBar_Tacgnol: ui_.progressBarTacgnol->setChecked(true); break;
	}
}

void ConfigureDialog::pageChanged(const QString& text)
{
	ui_.pageTitle_->setText("<b>" + text + "</b>");
}
