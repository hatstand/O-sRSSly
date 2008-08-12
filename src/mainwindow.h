#include <QMainWindow>
#include <QModelIndex>

#include "ui_mainwindow.h"

class ConfigureDialog;
class FeedsModel;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

public slots:
	void showConfigure();

private slots:
	void subscriptionSelected(const QModelIndex& index);
	
private:
	Ui_MainWindow ui_;

	FeedsModel* feeds_model_;
	QMenu* feed_menu_;

	ConfigureDialog* configure_dialog_;

};
