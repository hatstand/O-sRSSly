#include <QMainWindow>
#include <QModelIndex>
#include <QSystemTrayIcon>

#include "ui_mainwindow.h"

class AboutBox;
class ConfigureDialog;
class FeedsModel;
class ReaderApi;

class QSortFilterProxyModel;

namespace Spawn {
	class View;
	class Manager;
}

class QWebView;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

public slots:
	void showConfigure();

private slots:
	void seeOriginal();
	void subscriptionSelected(const QModelIndex& index);
	void entrySelected(const QModelIndex& index);

	void entryModelDeleted(QObject* object);

	void externalLinkClicked(const QUrl& url);
	void titleChanged(const QString& title);
	void statusBarMessage(const QString& message);
	void iconChanged();
	void webclipClicked();
	void closeTab();
	void tabChanged(int tab);
	void loadProgress(int progress);
	//void loadProgress(int loadProgress);
	void showUnreadOnly(bool enable);
	
	void apiProgress(int value, int total);
	void newUnreadItems(int count);
	
	void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	void toggleWindowVisibility();

	void xpathSet(const QString& xpath);

	void markAllRead();
	void shareItem();
	void saveState();

	void about();
	void aboutQt();

	void configureDone(int);
	void subscribe();

private:
	void updateProgressBar();
	void closeEvent(QCloseEvent* event);
	
	Ui_MainWindow ui_;
	QSystemTrayIcon* tray_icon_;
	QMenu* tray_menu_;
	QAction* toggle_visiblity_action_;
	int current_unread_;

	FeedsModel* feeds_model_;
	QSortFilterProxyModel* sorted_entries_;
	QMenu* feed_menu_;
	LongCatBar* web_progress_bar_;
	QMap<QWidget*, int> web_progress_;

	ConfigureDialog* configure_dialog_;
	AboutBox* about_box_;

	bool webclipping_;
	bool unread_only_;

	QModelIndex current_contents_;

	static const QString kTrayToolTip;

	ReaderApi* api_;
	
#ifdef USE_SPAWN
	Spawn::Manager* spawn_manager_;
	Spawn::View* contents_;
#else
	QWebView* contents_;
#endif
};
