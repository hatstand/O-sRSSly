#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"
#include "settings.h"
#include "browser.h"
#include "xmlutils.h"

#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QShortcut>
#include <QTextDocument>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	  feeds_model_(new FeedsModel(this)),
	  sorted_entries_(0),
	  feed_menu_(new QMenu(this)),
	  web_progress_bar_(new LongCatBar(this)),
	  configure_dialog_(new ConfigureDialog(this)),
	  webclipping_(false),
	  unread_only_(false)
{
	ui_.setupUi(this);
	
	//menuBar()->hide();
	statusBar()->hide();
	ui_.subtitleStack_->hide();
	ui_.date_->hide();

	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionConfigure_, SIGNAL(activated()), SLOT(showConfigure()));
	ui_.configure_->setDefaultAction(ui_.actionConfigure_);

	connect(ui_.action_refresh_, SIGNAL(activated()), feeds_model_, SLOT(fetchMore()));
	ui_.refresh_->setDefaultAction(ui_.action_refresh_);
	
	ui_.title_->setText("<b>Welcome to Feeder</b>");
	ui_.contents_->setUrl(QUrl("qrc:/welcome.html"));
	ui_.contents_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
	connect(ui_.contents_, SIGNAL(linkClicked(const QUrl&)),
		SLOT(externalLinkClicked(const QUrl&)));

	ui_.feeds_->setModel(feeds_model_);
	ui_.feeds_->hideColumn(1);
	ui_.feeds_->hideColumn(2);

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

	// Progress bar for tabs
	web_progress_bar_->setMinimum(0);
	web_progress_bar_->setMaximum(100);
	ui_.tabs_->setCornerWidget(web_progress_bar_, Qt::BottomRightCorner);
	connect(ui_.contents_, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
	
	// General progress bar
	connect(feeds_model_, SIGNAL(progressChanged(int, int)), SLOT(apiProgress(int, int)));

	connect(ui_.actionUnreadOnly_, SIGNAL(triggered(bool)), SLOT(showUnreadOnly(bool)));
	
	// Prompt the user for google account details
	if (Settings::instance()->googleUsername().isNull())
		if (configure_dialog_->exec() == QDialog::Rejected)
			exit(0);

	unread_only_ = Settings::instance()->unreadOnly();
	ui_.actionUnreadOnly_->setChecked(unread_only_);
	
	connect(ui_.actionWebclip_, SIGNAL(triggered(bool)), SLOT(webclipClicked()));
	ui_.webclip_->setDefaultAction(ui_.actionWebclip_);

	ui_.actionWebclip_->setEnabled(false);
	connect(ui_.contents_, SIGNAL(loadFinished(bool)), ui_.actionWebclip_, SLOT(setEnabled(bool)));
	connect(ui_.contents_, SIGNAL(xpathSet(const QString&)), SLOT(xpathSet(const QString&)));

	ui_.title_->setTextElideMode(Qt::ElideRight);
}

MainWindow::~MainWindow() {
}

void MainWindow::showConfigure() {
	configure_dialog_->show();
}

void MainWindow::subscriptionSelected(const QModelIndex& index) {
	QAbstractItemModel* model = feeds_model_->getEntries(index);
	qDebug() << model->rowCount(QModelIndex()) << model->columnCount(QModelIndex()) << model->index(0, 0);
	if (model) {
		if (!sorted_entries_) {
			sorted_entries_ = new QSortFilterProxyModel(this);
			sorted_entries_->setDynamicSortFilter(true);
			ui_.entries_->setModel(sorted_entries_);
			sorted_entries_->setFilterKeyColumn(TreeItem::Column_Read);
			// Filter on read/unread.
			if (unread_only_) {
				sorted_entries_->setFilterFixedString("false");
			}
		}

		sorted_entries_->setSourceModel(model);
		sorted_entries_->sort(2, Qt::DescendingOrder);
		connect(model, SIGNAL(destroyed(QObject*)), SLOT(entryModelDeleted(QObject*)));
	}
}

void MainWindow::entrySelected(const QModelIndex& index) {
	qDebug() << __PRETTY_FUNCTION__;
	
	QUrl link(index.sibling(index.row(), TreeItem::Column_Link).data().toUrl());
	QDateTime date(index.sibling(index.row(), TreeItem::Column_Date).data().toDateTime());

	current_contents_ = index;

	ui_.title_->setText("<b>" + index.sibling(index.row(), TreeItem::Column_Title).data(Qt::DisplayRole).toString() + "</b>");
	ui_.date_->setText(date.toString());
	ui_.date_->show();

	QString summary = index.sibling(index.row(), TreeItem::Column_Summary).data().toString();
	QString content = index.sibling(index.row(), TreeItem::Column_Content).data().toString();
	QString real_id = index.sibling(index.row(), TreeItem::Column_Id).data().toString();

	switch (Settings::instance()->behaviour(real_id)) {
		case Settings::Behaviour_Auto:
			if (summary.isEmpty() && content.isEmpty()) {
				ui_.actionWebclip_->setEnabled(true);
				ui_.contents_->setUrl(link);
				ui_.subtitleStack_->setCurrentIndex(1);
			} else {
				ui_.actionWebclip_->setEnabled(false);
				if (content.isEmpty())
					ui_.contents_->setContent(summary.toUtf8());
				else
					ui_.contents_->setContent(content.toUtf8());

				ui_.subtitleStack_->setCurrentIndex(0);
				ui_.seeOriginal_->setText("<a href=\"" + Qt::escape(link.toString()) + "\">See original</a>");
			}
			ui_.subtitleStack_->show();
			break;
			
		case Settings::Behaviour_ShowInline:
			ui_.actionWebclip_->setEnabled(false);
			ui_.contents_->setContent((content.isEmpty() ? summary : content).toUtf8());
			ui_.subtitleStack_->setCurrentIndex(0);
			ui_.seeOriginal_->setText("<a href=\"" + Qt::escape(link.toString()) + "\">See original</a>");
			ui_.subtitleStack_->show();
			break;
		
		case Settings::Behaviour_OpenInBrowser:
			ui_.contents_->setUrl(link);
			ui_.actionWebclip_->setEnabled(true);
			ui_.subtitleStack_->hide();
			break;

		case Settings::Behaviour_Webclip:
			ui_.contents_->setUrl(link);
			ui_.contents_->getXpath(index.sibling(index.row(), TreeItem::Column_Xpath).data().toString());
			ui_.actionWebclip_->setEnabled(false);
			break;
	}
	
	ui_.tabs_->setCurrentIndex(0);

	// Set read locally.
	const_cast<QAbstractItemModel*>(index.model())->setData(index.sibling(index.row(), TreeItem::Column_Read), true);
}

void MainWindow::externalLinkClicked(const QUrl& url) {
	qDebug() << __PRETTY_FUNCTION__;
	
	Browser* browser = new Browser(this);
	connect(browser, SIGNAL(titleChanged(const QString&)), SLOT(titleChanged(const QString&)));
	connect(browser, SIGNAL(statusBarMessage(const QString&)), SLOT(statusBarMessage(const QString&)));
	connect(browser, SIGNAL(iconChanged()), SLOT(iconChanged()));
	connect(browser, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
	
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
	widget->deleteLater();
	web_progress_.remove(widget);
	updateProgressBar();
}

void MainWindow::tabChanged(int tab) {
	ui_.actionCloseTab_->setEnabled(tab != 0);
	updateProgressBar();
}

void MainWindow::showUnreadOnly(bool enable) {
	unread_only_ = enable;
	Settings::instance()->setUnreadOnly(unread_only_);

	if (sorted_entries_) {
		if (unread_only_) {
			sorted_entries_->setFilterFixedString("false");
		} else {
			sorted_entries_->setFilterFixedString(QString::null);
		}
	}
}

void MainWindow::loadProgress(int progress) {
	QWidget* web_view(qobject_cast<QWidget*>(sender()));
	web_progress_[web_view] = progress;
	
	updateProgressBar();
}

void MainWindow::updateProgressBar() {
	Browser* browser = qobject_cast<Browser*>(ui_.tabs_->currentWidget());
	QWidget* web_view;
	if (browser)
		web_view = browser;
	else
		web_view = ui_.contents_;
	
	int value = 100;
	if (web_progress_.contains(web_view)) {
		value = web_progress_[web_view];
	}
	
	web_progress_bar_->setValue(value);
	web_progress_bar_->setHidden(value == 100);
}

void MainWindow::xpathSet(const QString& xpath) {
	if (current_contents_.isValid()) {
		qDebug() << __PRETTY_FUNCTION__;
		const_cast<QAbstractItemModel*>(current_contents_.model())->setData(
			current_contents_.sibling(current_contents_.row(), TreeItem::Column_Xpath), xpath);
	}
}

void MainWindow::apiProgress(int value, int total) {
	ui_.progress_stack_->setCurrentIndex(value == total ? 1 : 0);
	ui_.progress_->setMaximum(total);
	ui_.progress_->setValue(value);
}
