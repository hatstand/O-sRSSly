#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include "ui_configuredialog.h"

#include <QDialog>
#include <QTimer>

class ReaderApi;
class Settings;

class QMovie;

class ConfigureDialog : public QDialog
{
	Q_OBJECT
public:
	ConfigureDialog(ReaderApi* api, QWidget* parent);
	~ConfigureDialog();

public slots:
	void accept();
	void show();
	void open();

private slots:
	void pageChanged(const QString& text);
	void loggedIn(bool);
	void textChanged();
	void accountUpdated();

private:
	void populateData();
	
	Ui::ConfigureDialog ui_;
	Settings* settings_;
	ReaderApi* api_;

	QTimer timer_;

	QMovie* spinner_;
};

#endif
