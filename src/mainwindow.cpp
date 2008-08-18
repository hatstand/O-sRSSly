#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"

#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), feeds_model_(new FeedsModel(this)),
	  sorted_entries_(0), feed_menu_(new QMenu(this)),
	  configure_dialog_(new ConfigureDialog(this)) {
	ui_.setupUi(this);

	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionSettings, SIGNAL(activated()), SLOT(showConfigure()));

	ui_.contents_->setContent("Hello, World!");
	ui_.contents_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
	connect(ui_.contents_, SIGNAL(linkClicked(const QUrl&)),
		SLOT(externalLinkClicked(const QUrl&)));

	ui_.feeds_->setModel(feeds_model_);

	connect(ui_.feeds_, SIGNAL(activated(const QModelIndex&)),
		SLOT(subscriptionSelected(const QModelIndex&)));

	connect(ui_.entries_, SIGNAL(activated(const QModelIndex&)),
		SLOT(entrySelected(const QModelIndex&)));
}

MainWindow::~MainWindow() {

}

void MainWindow::showConfigure() {
	configure_dialog_->show();
}

void MainWindow::subscriptionSelected(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	QAbstractItemModel* model = feeds_model_->getEntries(index);
	if (model) {
		if (!sorted_entries_) {
			sorted_entries_ = new QSortFilterProxyModel(this);
			ui_.entries_->setModel(sorted_entries_);
		}

		sorted_entries_->setSourceModel(model);
		ui_.entries_->resizeColumnToContents(0);
	}
}

void MainWindow::entrySelected(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	
	QModelIndex real_index = sorted_entries_->mapToSource(index);

	const TreeItem* item = static_cast<const TreeItem*>(real_index.model());

	ui_.contents_->setContent(item->summary(real_index).toUtf8());

	// Set read on server.
	const AtomEntry& e = item->entry(real_index);
	feeds_model_->setRead(e);

	// Set read locally.
	const_cast<TreeItem*>(item)->setRead(real_index);
}

void MainWindow::externalLinkClicked(const QUrl& url) {
	qDebug() << __PRETTY_FUNCTION__;
	
	QWebView* view = new QWebView(this);
	int index = ui_.tabs_->addTab(view, url.toString());
	ui_.tabs_->setCurrentIndex(index);

	view->setUrl(url);
}
