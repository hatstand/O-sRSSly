#include <QMainWindow>
#include <QModelIndex>

#include "ui_mainwindow.h"

class ConfigureDialog;
class FeedsModel;
class QSortFilterProxyModel;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

public slots:
	void showConfigure();

private slots:
	void seeOriginal(const QString& url);
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
	//void loadProgress(int loadProgress);
	
private:
	Ui_MainWindow ui_;

	FeedsModel* feeds_model_;
	QSortFilterProxyModel* sorted_entries_;
	QMenu* feed_menu_;

	ConfigureDialog* configure_dialog_;

	bool webclipping_;
};
