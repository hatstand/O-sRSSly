#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"
#include "settings.h"
#include "browser.h"

#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QShortcut>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	  feeds_model_(new FeedsModel(this)),
	  sorted_entries_(0),
	  feed_menu_(new QMenu(this)),
	  configure_dialog_(new ConfigureDialog(this))
{
	ui_.setupUi(this);

	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionSettings, SIGNAL(activated()), SLOT(showConfigure()));
	
	ui_.contents_->setUrl(QUrl("qrc:/welcome.html"));
	ui_.contents_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
	connect(ui_.contents_, SIGNAL(linkClicked(const QUrl&)),
		SLOT(externalLinkClicked(const QUrl&)));

	ui_.feeds_->setModel(feeds_model_);

	connect(ui_.feeds_, SIGNAL(activated(const QModelIndex&)),
		SLOT(subscriptionSelected(const QModelIndex&)));

	connect(ui_.entries_, SIGNAL(activated(const QModelIndex&)),
		SLOT(entrySelected(const QModelIndex&)));
	
	// Connect the up and down actions
	connect(ui_.actionPrevious_, SIGNAL(triggered(bool)), ui_.entries_, SLOT(previous()));
	connect(ui_.actionNext_, SIGNAL(triggered(bool)), ui_.entries_, SLOT(next()));
	ui_.up_->setDefaultAction(ui_.actionPrevious_);
	ui_.down_->setDefaultAction(ui_.actionNext_);
	
	// Extra shortcuts for up and down
	connect(new QShortcut(Qt::Key_J, this), SIGNAL(activated()), ui_.actionNext_, SLOT(trigger()));
	connect(new QShortcut(Qt::Key_K, this), SIGNAL(activated()), ui_.actionPrevious_, SLOT(trigger()));
	
	// Prompt the user for google account details
	if (Settings::instance()->googleUsername().isNull())
		if (configure_dialog_->exec() == QDialog::Rejected)
			exit(0);
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
		connect(model, SIGNAL(destroyed(QObject*)), SLOT(entryModelDeleted(QObject*)));
	}
}

void MainWindow::entrySelected(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	
	QModelIndex real_index = sorted_entries_->mapToSource(index);

	const TreeItem* item = static_cast<const TreeItem*>(real_index.model());

	ui_.contents_->setContent(item->summary(real_index).toUtf8());
	ui_.title_->setText("<b>" + item->data(real_index, Qt::DisplayRole).toString() + "</b>");

	// Set read locally.
	const_cast<TreeItem*>(item)->setRead(real_index);
}

void MainWindow::externalLinkClicked(const QUrl& url) {
	qDebug() << __PRETTY_FUNCTION__;
	
	Browser* browser = new Browser(this);
	connect(browser, SIGNAL(titleChanged(const QString&)), SLOT(titleChanged(const QString&)));
	connect(browser, SIGNAL(statusBarMessage(const QString&)), SLOT(statusBarMessage(const QString&)));
	connect(browser, SIGNAL(iconChanged()), SLOT(iconChanged()));
	
	int index = ui_.tabs_->addTab(browser, url.toString());
	ui_.tabs_->setCurrentIndex(index);

	browser->setUrl(url);
}

void MainWindow::entryModelDeleted(QObject* object) {
	sorted_entries_->setSourceModel(0);
}

void MainWindow::titleChanged(const QString& title) {
	Browser* browser = static_cast<Browser*>(sender());
	int index = ui_.tabs_->indexOf(browser);
	
	ui_.tabs_->setTabText(index, title);
}

void MainWindow::statusBarMessage(const QString& message) {
	Browser* browser = static_cast<Browser*>(sender());
	int index = ui_.tabs_->indexOf(browser);
	
	if (index != ui_.tabs_->currentIndex())
		return;
	
	statusBar()->showMessage(message);
}

void MainWindow::iconChanged() {
	Browser* browser = static_cast<Browser*>(sender());
	int index = ui_.tabs_->indexOf(browser);
	
	ui_.tabs_->setTabIcon(index, browser->icon());
}

