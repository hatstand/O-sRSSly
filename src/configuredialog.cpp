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
	Settings::ProgressBarStyle progressBarStyle = Settings::ProgressBar_Normal;
	if (ui_.progressBarLongcat->isChecked())  progressBarStyle = Settings::ProgressBar_Longcat;
	if (ui_.progressBarTacgnol->isChecked())  progressBarStyle = Settings::ProgressBar_Tacgnol;
	
	settings_->setGoogleAccount(ui_.user_->text(), ui_.password_->text());
	settings_->setProgressBarStyle(progressBarStyle);
	
	settings_->setShowTrayIcon(ui_.show_tray_icon_->isChecked());
	settings_->setStartMinimized(ui_.start_minimized_->isChecked());
	settings_->setCheckNew(ui_.check_new_->isChecked());
	settings_->setCheckNewInterval(ui_.check_new_interval_->value());
	settings_->setShowBubble(ui_.show_bubble_->isChecked());

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
	
	ui_.show_tray_icon_->setChecked(settings_->showTrayIcon());
	ui_.start_minimized_->setChecked(settings_->startMinimized());
	ui_.check_new_->setChecked(settings_->checkNew());
	ui_.check_new_interval_->setValue(settings_->checkNewInterval());
	ui_.show_bubble_->setChecked(settings_->showBubble());
}

void ConfigureDialog::pageChanged(const QString& text)
{
	ui_.pageTitle_->setText("<b>" + text + "</b>");
}
