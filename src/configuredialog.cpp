#include "configuredialog.h"
#include "readerapi.h"
#include "settings.h"

#include <QMovie>

ConfigureDialog::ConfigureDialog(ReaderApi* api, QWidget* parent)
	: QDialog(parent),
	  settings_(Settings::instance()),
	  api_(api),
	  spinner_(new QMovie(":/spinner.gif", QByteArray(), this))
{
	ui_.setupUi(this);
	
	connect(ui_.pageList_, SIGNAL(currentTextChanged(const QString&)), SLOT(pageChanged(const QString&)));
	ui_.pageList_->setCurrentRow(0);

	connect(ui_.user_, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));
	connect(ui_.password_, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));

	timer_.setSingleShot(true);
	connect(&timer_, SIGNAL(timeout()), SLOT(accountUpdated()));

	connect(api_, SIGNAL(loggedIn(bool)), SLOT(loggedIn(bool)));

	ui_.login_status_->hide();
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
	if (ui_.password_->text().isEmpty() && !ui_.user_->text().isEmpty()) {
		ui_.password_->setFocus(Qt::OtherFocusReason);
	}
	QDialog::show();
}

void ConfigureDialog::open()
{
	populateData();
	QDialog::open();
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

void ConfigureDialog::loggedIn(bool success)
{
	if (success) {
		ui_.login_status_image_->setPixmap(QPixmap(":/login_ok.png"));
		ui_.login_status_label_->setText("Login ok!");
	}
	else {
		ui_.login_status_image_->setPixmap(QPixmap(":/login_bad.png"));
		ui_.login_status_label_->setText("Login failed");
	}
}

void ConfigureDialog::textChanged() {
	timer_.start(1000);
}

void ConfigureDialog::accountUpdated() {
	// User changed account details. Let's try logging in.
	QString username = ui_.user_->text();
	QString password = ui_.password_->text();

	if (!username.isEmpty() && !password.isEmpty()) {
		api_->login(username, password);

		ui_.login_status_image_->setMovie(spinner_);
		ui_.login_status_label_->setText("Logging in...");
		ui_.login_status_->show();
		spinner_->start();
	}
}
