#include <QMainWindow>

#include "ui_mainwindow.h"

class FeedsModel;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

	
private:
	Ui_MainWindow ui_;

	FeedsModel* feeds_model_;
	QMenu* feed_menu_;

};
