#ifndef SERIOUSAPP_H
#define SERIOUSAPP_H

#include <QApplication>
#include <QPixmap>

class SeriousApp : public QApplication {
	Q_OBJECT
public:
	SeriousApp(int& argc, char** argv);
	virtual void commitData(QSessionManager& session);
	virtual void saveState(QSessionManager& session);

public slots:
	void setUnreadItems(int n);

private:
	void generateImage(const QString& s);

	QImage original_image_;
	QPixmap icon_;
};

#endif
