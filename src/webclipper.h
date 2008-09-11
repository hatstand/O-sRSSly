#ifndef WEBCLIPPER_H
#define WEBCLIPPER_H

#include <boost/shared_ptr.hpp>

#include <QImage>
#include <QReadWriteLock>
#include <QString>
#include <QThread>
#include <QUrl>
#include <QWebPage>

using boost::shared_ptr;

class Webclipper : public QThread {
	Q_OBJECT
public:
	Webclipper(const QUrl& url, const QString& xpath, QObject* parent = 0);
	virtual void run();
	QImage snapshot();
	shared_ptr<QWebPage> webpage();

private slots:
	void loaded(bool ok);
private:
	shared_ptr<QWebPage> webpage_; 
	QUrl url_;
	QString xpath_;

	QImage snapshot_;
	QReadWriteLock image_mutex_;
	QReadWriteLock webpage_mutex_;
};

#endif
