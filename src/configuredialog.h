#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include "ui_configuredialog.h"

#include <QDialog>

class ConfigureDialog : public QDialog, public Ui::ConfigureDialog {
	Q_OBJECT
public:
	ConfigureDialog(QWidget* parent);
	~ConfigureDialog();

public slots:
	void accept();
	void show();
};

#endif
