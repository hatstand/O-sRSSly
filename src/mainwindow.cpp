#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"
#include "settings.h"
#include "browser.h"
#include "xmlutils.h"

#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QShortcut>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	  feeds_model_(new FeedsModel(this)),
	  sorted_entries_(0),
	  feed_menu_(new QMenu(this)),
	  configure_dialog_(new ConfigureDialog(this)),
	  webclipping_(false)
{
	ui_.setupUi(this);
	
	menuBar()->hide();
	statusBar()->hide();
	ui_.subtitleStack_->hide();
	ui_.date_->hide();

	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionConfigure_, SIGNAL(activated()), SLOT(showConfigure()));
	ui_.configure_->setDefaultAction(ui_.actionConfigure_);
	
	ui_.contents_->setUrl(QUrl("qrc:/welcome.html"));
	ui_.contents_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
	connect(ui_.contents_, SIGNAL(linkClicked(const QUrl&)),
		SLOT(externalLinkClicked(const QUrl&)));

	ui_.feeds_->setModel(feeds_model_);
	ui_.feeds_->hideColumn(1);

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
	
	connect(ui_.entries_, SIGNAL(canGoUpChanged(bool)), ui_.actionPrevious_, SLOT(setEnabled(bool)));
	connect(ui_.entries_, SIGNAL(canGoDownChanged(bool)), ui_.actionNext_, SLOT(setEnabled(bool)));
	
	// Other things on the title bar
	connect(ui_.seeOriginal_, SIGNAL(linkActivated(const QString&)), SLOT(seeOriginal(const QString&)));
	
	// Close button for tabs
	QToolButton* closeTabButton = new QToolButton(this);
	closeTabButton->setDefaultAction(ui_.actionCloseTab_);
	ui_.tabs_->setCornerWidget(closeTabButton, Qt::BottomLeftCorner);
	connect(ui_.actionCloseTab_, SIGNAL(triggered(bool)), SLOT(closeTab()));
	connect(ui_.tabs_, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
	
	// Prompt the user for google account details
	if (Settings::instance()->googleUsername().isNull())
		if (configure_dialog_->exec() == QDialog::Rejected)
			exit(0);
	
	connect(ui_.actionWebclip_, SIGNAL(triggered(bool)), SLOT(webclipClicked()));
	ui_.webclip_->setDefaultAction(ui_.actionWebclip_);
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
			sorted_entries_->setDynamicSortFilter(true);
			ui_.entries_->setModel(sorted_entries_);
		}

		sorted_entries_->setSourceModel(model);
		sorted_entries_->sort(2, Qt::DescendingOrder);
		connect(model, SIGNAL(destroyed(QObject*)), SLOT(entryModelDeleted(QObject*)));
	}
}

void MainWindow::entrySelected(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	
	QModelIndex real_index = sorted_entries_->mapToSource(index);
	QUrl link(real_index.sibling(real_index.row(), 5).data().toUrl());
	QDateTime date(real_index.sibling(real_index.row(), 2).data().toDateTime());
	const TreeItem* item = static_cast<const TreeItem*>(real_index.model());

	ui_.title_->setText("<b>" + item->data(real_index, Qt::DisplayRole).toString() + "</b>");
	ui_.date_->setText(date.toString());
	ui_.date_->show();
	
	switch (Settings::instance()->behaviour(item->real_id(real_index))) {
		case Settings::Auto:
			if (item->summary(real_index).length() == 0) {
				ui_.contents_->setUrl(link);
				ui_.subtitleStack_->setCurrentIndex(1);
			} else {
				ui_.contents_->setContent(item->summary(real_index).toUtf8());
				ui_.subtitleStack_->setCurrentIndex(0);
				ui_.seeOriginal_->setText("<a href=\"" + XmlUtils::escaped(link.toString()) + "\">See original</a>");
			}
			ui_.subtitleStack_->show();
			break;
			
		case Settings::ShowInline:
			ui_.contents_->setContent(item->summary(real_index).toUtf8());
			ui_.subtitleStack_->setCurrentIndex(0);
			ui_.seeOriginal_->setText("<a href=\"" + XmlUtils::escaped(link.toString()) + "\">See original</a>");
			ui_.subtitleStack_->show();
			break;
		
		case Settings::OpenInBrowser:
			ui_.contents_->setUrl(link);
			ui_.subtitleStack_->hide();
			break;
	}
	
	ui_.tabs_->setCurrentIndex(0);

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

void MainWindow::seeOriginal(const QString& url) {
	externalLinkClicked(QUrl(url));
}

void MainWindow::webclipClicked() {
	qDebug() << __PRETTY_FUNCTION__;
	webclipping_ = !webclipping_;

	ui_.contents_->setWebclipping(webclipping_);
}

void MainWindow::closeTab() {
	int index = ui_.tabs_->currentIndex();
	if (index == 0)
		return;
	
	QWidget* widget = ui_.tabs_->widget(index);
	ui_.tabs_->removeTab(index);
	delete widget;
}

void MainWindow::tabChanged(int tab) {
	ui_.actionCloseTab_->setEnabled(tab != 0);
}
