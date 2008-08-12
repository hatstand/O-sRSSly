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

	TreeItem* root = feeds_model_->root();
	FeedItem* item1 = new FeedItem(root, QString("Slashdot"), QUrl("http://slashdot.org"));
	root->appendChild(item1);

	FolderItem* folder1 = new FolderItem(root, QString("IT"));
	root->appendChild(folder1);

	FeedItem* item2 = new FeedItem(folder1, QString("The Reg"),
		QUrl("http://www.theregister.co.uk/headlines.rss"));
	folder1->appendChild(item2);
}

MainWindow::~MainWindow() {

}

void MainWindow::showConfigure() {
	configure_dialog_->show();
}
