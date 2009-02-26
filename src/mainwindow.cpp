#include "config.h"
#include "configuredialog.h"
#include "feedsmodel.h"
#include "mainwindow.h"
#include "settings.h"
#include "browser.h"
#include "xmlutils.h"
#include "about.h"

#ifdef USE_SPAWN
#include <spawn/view.h>
#include <spawn/manager.h>
#endif

#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>
#include <QSizeGrip>
#include <QTextDocument>
#include <QSystemTrayIcon>
#include <QWebSettings>
#include <QWebView>

const QString MainWindow::kTrayToolTip = TITLE " -- \"Imma in ur dock aggregating ur feedz\"";

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	  tray_icon_(new QSystemTrayIcon(this)),
	  tray_menu_(new QMenu(this)),
	  current_unread_(0),
	  feeds_model_(new FeedsModel(this)),
	  sorted_entries_(0),
	  feed_menu_(new QMenu(this)),
	  web_progress_bar_(new LongCatBar(this)),
	  configure_dialog_(new ConfigureDialog(this)),
	  about_box_(NULL),
	  webclipping_(false),
	  unread_only_(false)
#ifdef USE_SPAWN
	  ,spawn_manager_(new Spawn::Manager(this))
#endif
{
#ifdef Q_OS_DARWIN
	new QSizeGrip(this);
#endif
	ui_.setupUi(this);

	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
	
	toggle_visiblity_action_ = tray_menu_->addAction("Hide", this, SLOT(toggleWindowVisibility()));
	tray_menu_->addAction(ui_.actionQuit);
	connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	
	tray_icon_->setIcon(windowIcon());
	tray_icon_->setContextMenu(tray_menu_);
	tray_icon_->setToolTip(kTrayToolTip);
	tray_icon_->show();
	connect(feeds_model_, SIGNAL(newUnreadItems(int)), SLOT(newUnreadItems(int)));
	
	//menuBar()->hide();
	statusBar()->hide();
	ui_.subtitleStack_->hide();
	ui_.date_->hide();

	connect(ui_.actionQuit, SIGNAL(activated()), SLOT(saveState()));
	connect(ui_.actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(ui_.actionConfigure_, SIGNAL(activated()), SLOT(showConfigure()));
	ui_.configure_->setDefaultAction(ui_.actionConfigure_);

	connect(ui_.action_refresh_, SIGNAL(activated()), feeds_model_, SLOT(fetchMore()));
	ui_.refresh_->setDefaultAction(ui_.action_refresh_);

	QFont bold_font;
	bold_font.setBold(true);
	ui_.title_->setFont(bold_font);
	ui_.title_->setText("Welcome to " TITLE);
	
#ifdef USE_SPAWN
	contents_ = new Spawn::View(spawn_manager_, ui_.contents_tab_);
#else
	contents_ = new QWebView(this);
#endif
	ui_.contents_tab_->layout()->removeWidget(ui_.view_container_);
	delete ui_.view_container_;
	ui_.contents_tab_->layout()->addWidget(contents_);
	
	contents_->setUrl(QUrl("qrc:/welcome.html"));
#ifdef USE_SPAWN
	contents_->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
#else
	contents_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
#endif
	
	connect(contents_, SIGNAL(linkClicked(const QUrl&)),
		SLOT(externalLinkClicked(const QUrl&)));

	ui_.feeds_->setModel(feeds_model_);
	for (int i=1 ; i<feeds_model_->columnCount() ; ++i)
		ui_.feeds_->hideColumn(i);
	ui_.feeds_->expandAll();
	connect(feeds_model_, SIGNAL(modelReset()), ui_.feeds_, SLOT(expandAll()));
	connect(feeds_model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), ui_.feeds_, SLOT(expandAll()));

	connect(ui_.feeds_, SIGNAL(activated(const QModelIndex&)),
		SLOT(subscriptionSelected(const QModelIndex&)));

	connect(ui_.entries_, SIGNAL(activated(const QModelIndex&)),
		SLOT(entrySelected(const QModelIndex&)));
	
	// Connect the up and down actions
	connect(ui_.actionPrevious_, SIGNAL(triggered(bool)), ui_.entries_, SLOT(previous()));
	connect(ui_.actionNext_, SIGNAL(triggered(bool)), ui_.entries_, SLOT(next()));
	ui_.up_->setDefaultAction(ui_.actionPrevious_);
	ui_.down_->setDefaultAction(ui_.actionNext_);

	connect(ui_.actionShare_, SIGNAL(triggered(bool)), SLOT(shareItem()));
	ui_.share_item_->setDefaultAction(ui_.actionShare_);
	ui_.actionShare_->setEnabled(false);
	
	// Extra shortcuts for up and down
	connect(new QShortcut(Qt::Key_J, this), SIGNAL(activated()), ui_.actionNext_, SLOT(trigger()));
	connect(new QShortcut(Qt::Key_K, this), SIGNAL(activated()), ui_.actionPrevious_, SLOT(trigger()));
	
	connect(ui_.entries_, SIGNAL(canGoUpChanged(bool)), ui_.actionPrevious_, SLOT(setEnabled(bool)));
	connect(ui_.entries_, SIGNAL(canGoDownChanged(bool)), ui_.actionNext_, SLOT(setEnabled(bool)));

	// Mark as read
	connect(ui_.actionMarkRead_, SIGNAL(triggered(bool)), this, SLOT(markAllRead()));
	
	// Other things on the title bar
	connect(ui_.seeOriginal_, SIGNAL(linkActivated(const QString&)), SLOT(seeOriginal()));
	connect(new QShortcut(Qt::Key_V, this), SIGNAL(activated()), SLOT(seeOriginal()));
	
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
	connect(contents_, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
	
	// General progress bar
	connect(feeds_model_, SIGNAL(progressChanged(int, int)), SLOT(apiProgress(int, int)));

	connect(ui_.actionUnreadOnly_, SIGNAL(triggered(bool)), SLOT(showUnreadOnly(bool)));

	// About stuff
	connect(ui_.action_about_, SIGNAL(triggered(bool)), SLOT(about()));
	connect(ui_.action_about_qt_, SIGNAL(triggered(bool)), SLOT(aboutQt()));
	
	// Prompt the user for google account details
	if (Settings::instance()->googleUsername().isNull() || Settings::instance()->googlePassword().isNull())
		if (configure_dialog_->exec() == QDialog::Rejected)
			exit(0);

	unread_only_ = Settings::instance()->unreadOnly();
	ui_.actionUnreadOnly_->setChecked(unread_only_);
	
	connect(ui_.actionWebclip_, SIGNAL(triggered(bool)), SLOT(webclipClicked()));
	ui_.webclip_->setDefaultAction(ui_.actionWebclip_);

	ui_.actionWebclip_->setEnabled(false);
	connect(contents_, SIGNAL(loadFinished(bool)), ui_.actionWebclip_, SLOT(setEnabled(bool)));
	// TODO: Fixme
	//connect(contents_, SIGNAL(xpathSet(const QString&)), SLOT(xpathSet(const QString&)));

	ui_.title_->setTextElideMode(Qt::ElideRight);
	QFont italic_font;
	italic_font.setItalic(true);
	//ui_.author_->setFont(italic_font);
	
	feeds_model_->googleAccountChanged();
	feeds_model_->load();
}

MainWindow::~MainWindow() {
}

void MainWindow::showConfigure() {
	configure_dialog_->show();
}

void MainWindow::subscriptionSelected(const QModelIndex& index) {
	QAbstractItemModel* model = feeds_model_->getEntries(index);
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
	ui_.actionShare_->setEnabled(true);
	
	QUrl link(index.sibling(index.row(), TreeItem::Column_Link).data().toUrl());
	QDateTime date(index.sibling(index.row(), TreeItem::Column_Date).data().toDateTime());

	current_contents_ = index;

	QString title = index.sibling(index.row(), TreeItem::Column_Title).data().toString();
	QString author = index.sibling(index.row(), TreeItem::Column_Author).data().toString();
	//if (!author.isEmpty())
	//	label_text += "<i> by " + author + "</i>";

	ui_.title_->setText(title);

	//ui_.author_->setText(author);

	ui_.date_->setText(date.toString());
	ui_.date_->show();

	QString summary = index.sibling(index.row(), TreeItem::Column_Summary).data().toString();
	QString content = index.sibling(index.row(), TreeItem::Column_Content).data().toString();
	QString real_id = index.sibling(index.row(), TreeItem::Column_Id).data().toString();

	switch (Settings::instance()->behaviour(real_id)) {
		case Settings::Behaviour_Auto:
			if (summary.isEmpty() && content.isEmpty()) {
				ui_.actionWebclip_->setEnabled(true);
				contents_->setUrl(link);
				ui_.subtitleStack_->setCurrentIndex(1);
			} else {
				ui_.actionWebclip_->setEnabled(false);
				if (content.isEmpty())
					contents_->setHtml(summary);
				else
					contents_->setHtml(content);

				ui_.subtitleStack_->setCurrentIndex(0);
				ui_.seeOriginal_->setText("<a href=\"" + Qt::escape(link.toString()) + "\">See original</a>");
			}
			ui_.subtitleStack_->show();
			break;
			
		case Settings::Behaviour_ShowInline:
			ui_.actionWebclip_->setEnabled(false);
			contents_->setHtml(content.isEmpty() ? summary : content);
			ui_.subtitleStack_->setCurrentIndex(0);
			ui_.seeOriginal_->setText("<a href=\"" + Qt::escape(link.toString()) + "\">See original</a>");
			ui_.subtitleStack_->show();
			break;
		
		case Settings::Behaviour_OpenInBrowser:
			contents_->setUrl(link);
			ui_.actionWebclip_->setEnabled(true);
			ui_.subtitleStack_->hide();
			break;

		case Settings::Behaviour_Webclip:
			QString xpath = index.sibling(index.row(), TreeItem::Column_Xpath).data().toString();
			contents_->setUrl(link);
			// TODO: Fixme
			//ui_.contents_->getXpath(xpath);
			ui_.actionWebclip_->setEnabled(false);
			break;
	}
	
	ui_.tabs_->setCurrentIndex(0);

	// Set read locally.
	const_cast<QAbstractItemModel*>(index.model())->setData(index.sibling(index.row(), TreeItem::Column_Read), true);
}

void MainWindow::externalLinkClicked(const QUrl& url) {
	qDebug() << __PRETTY_FUNCTION__;
	
#ifdef USE_SPAWN
	Browser* browser = new Browser(spawn_manager_, this);
#else
	Browser* browser = new Browser(this);
#endif
	connect(browser, SIGNAL(titleChanged(const QString&)), SLOT(titleChanged(const QString&)));
	connect(browser, SIGNAL(statusBarMessage(const QString&)), SLOT(statusBarMessage(const QString&)));
	connect(browser, SIGNAL(iconChanged()), SLOT(iconChanged()));
	connect(browser, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));
	
	int index = ui_.tabs_->addTab(browser, url.toString());
	ui_.tabs_->setCurrentIndex(index);

	browser->setUrl(url);
}

void MainWindow::entryModelDeleted(QObject* object) {
	Q_UNUSED(object);
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

void MainWindow::seeOriginal() {
	QUrl url = current_contents_.sibling(current_contents_.row(), TreeItem::Column_Link).data().toUrl();
	externalLinkClicked(url);
}

void MainWindow::webclipClicked() {
	qDebug() << __PRETTY_FUNCTION__;
	webclipping_ = !webclipping_;

	// TODO: Fixme
	//ui_.contents_->setWebclipping(webclipping_);
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
		web_view = contents_;
	
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

void MainWindow::newUnreadItems(int count) {
	if (count == current_unread_)
		return;
	
	current_unread_ = count;

	tray_icon_->setToolTip(kTrayToolTip + QString("\n%1 new items").arg(feeds_model_->unread()));

	bool p = count != 1;
	if (count != 0)
		tray_icon_->showMessage(TITLE, "There " + QString(p ? "are" : "is") + " " + QString::number(count) + " new unread item" + QString(p ? "s" : "") + ".");
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	// typical linux behavior is single clicking tray icon toggles the main window
	
#ifdef Q_WS_X11
	if (reason == QSystemTrayIcon::Trigger)
		toggleWindowVisibility();
#else
	if (reason == QSystemTrayIcon::DoubleClick)
		toggleWindowVisibility();
#endif
}

// Thanks last.fm
// svn://svn.audioscrobbler.net/clientside/Last.fm/tags/1.4.1.57486/src/container.cpp
#ifdef Q_WS_X11
    // includes only relevent to this function - please leave here :)
    #include <QX11Info>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
#endif

void MainWindow::toggleWindowVisibility()
{
	//TODO really we should check to see if the window itself is obscured?
	// hard to say as exact desire of user is a little hard to predict.
	// certainly we should raise the window if it isn't active as chances are it
	// is behind other windows
	
	if (isVisible()) {
		toggle_visiblity_action_->setText("Show");
		hide();
	} else {
		toggle_visiblity_action_->setText("Hide");
		#ifndef Q_WS_X11
		showNormal(), activateWindow(), raise();
		#else
		showNormal();
	
		//NOTE don't raise, as this won't work with focus stealing prevention
		//raise();
	
		QX11Info const i;
		Atom const _NET_ACTIVE_WINDOW = XInternAtom( i.display(), "_NET_ACTIVE_WINDOW", False);
	
		// this sends the correct demand for window activation to the Window
		// manager. Thus forcing window activation.
		///@see http://standards.freedesktop.org/wm-spec/wm-spec-1.3.html#id2506353
		XEvent e;
		e.xclient.type = ClientMessage;
		e.xclient.message_type = _NET_ACTIVE_WINDOW;
		e.xclient.display = i.display();
		e.xclient.window = winId();
		e.xclient.format = 32;
		e.xclient.data.l[0] = 1; // we are a normal application
		e.xclient.data.l[1] = i.appUserTime();
		e.xclient.data.l[2] = qApp->activeWindow() ? qApp->activeWindow()->winId() : 0;
		e.xclient.data.l[3] = 0l;
		e.xclient.data.l[4] = 0l;
	
		// we send to the root window per fdo NET spec
		XSendEvent( i.display(), i.appRootWindow(), false, SubstructureRedirectMask | SubstructureNotifyMask, &e );
		#endif
	}
}

void MainWindow::markAllRead() {
	qDebug() << __PRETTY_FUNCTION__;
	for (int row = 0; row < sorted_entries_->rowCount(QModelIndex()); ++row) {
		QModelIndex index = sorted_entries_->index(row, TreeItem::Column_Read);
		sorted_entries_->setData(index, QVariant(true));
	}
}

void MainWindow::shareItem() {
	qDebug() << __PRETTY_FUNCTION__ << current_contents_.sibling(current_contents_.row(), TreeItem::Column_Title).data().toString();
	const_cast<QAbstractItemModel*>(current_contents_.model())->setData(current_contents_.sibling(current_contents_.row(), TreeItem::Column_Shared), true);
}

void MainWindow::saveState() {
	qDebug() << __PRETTY_FUNCTION__;
	Settings::instance()->setGeometry(saveGeometry());
	Settings::instance()->commit();
}

void MainWindow::closeEvent(QCloseEvent* event) {
	qDebug() << __PRETTY_FUNCTION__;
	saveState();
	QMainWindow::closeEvent(event);
}

void MainWindow::about() {
	if (!about_box_)
		about_box_ = new AboutBox(this);

	about_box_->show();
}

void MainWindow::aboutQt() {
	QMessageBox::aboutQt(this);
}
