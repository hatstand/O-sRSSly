#include "feedsmodel.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), feeds_model_(new FeedsModel(this)),
	  feed_menu_(new QMenu(this)) {
	ui_.setupUi(this);

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
