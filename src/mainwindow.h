#include <QMainWindow>
#include <QModelIndex>

#include <boost/scoped_ptr.hpp>

#include "ui_mainwindow.h"

class ConfigureDialog;
class FeedsModel;
class Database;
class QSortFilterProxyModel;

using boost::scoped_ptr;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

public slots:
	void showConfigure();

private slots:
	void subscriptionSelected(const QModelIndex& index);
	void entrySelected(const QModelIndex& index);

	void entryModelDeleted(QObject* object);

	void externalLinkClicked(const QUrl& url);
	
private:
	Ui_MainWindow ui_;

	FeedsModel* feeds_model_;
	QSortFilterProxyModel* sorted_entries_;
	QMenu* feed_menu_;

	ConfigureDialog* configure_dialog_;
	
	scoped_ptr<Database> database_;

};
