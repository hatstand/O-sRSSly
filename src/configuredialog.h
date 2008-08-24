#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include "ui_configuredialog.h"

#include <QDialog>

class Settings;

class ConfigureDialog : public QDialog
{
	Q_OBJECT
public:
	ConfigureDialog(QWidget* parent);
	~ConfigureDialog();

public slots:
	void accept();
	void show();
	int exec();

private slots:
	void pageChanged(const QString& text);

private:
	void populateData();
	
	Ui::ConfigureDialog ui_;
	Settings* settings_;
};

#endif
