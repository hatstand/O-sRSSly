#ifndef WEBCLIPPER_H
#define WEBCLIPPER_H

#include <boost/scoped_ptr.hpp>

#include <QThread>
#include <QUrl>
#include <QWebPage>

using boost::scoped_ptr;

class Webclipper : public QThread {
	Q_OBJECT
public:
	Webclipper(const QUrl& url, QObject* parent = 0);
	virtual void run();

private slots:
	void loaded(bool ok);
private:
	scoped_ptr<QWebPage> webpage_; 
	QUrl url_;
};

#endif
