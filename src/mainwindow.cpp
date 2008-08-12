#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), feeds_model_(new FeedsModel(this)),
	  feed_menu_(new QMenu(this)), configure_dialog_(new ConfigureDialog(this)) {
	ui_.setupUi(this);

	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionSettings, SIGNAL(activated()), SLOT(showConfigure()));

	ui_.contents_->setPlainText("Hello, World!");

	ui_.feeds_->setModel(feeds_model_);
}

MainWindow::~MainWindow() {

}

void MainWindow::showConfigure() {
	configure_dialog_->show();
}
