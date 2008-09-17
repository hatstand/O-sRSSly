#ifndef WEBPAGE_PROCESS_H
#define WEBPAGE_PROCESS_H

#include <QObject>
#include <QSharedMemory>
#include <QSize>
#include <QUrl>
#include <QWebPage>

#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;

class QWebPage;

class WebpageProcess : public QObject {
	Q_OBJECT
public:
	WebpageProcess(const QUrl& url, QObject* parent = 0);
	QSize render();

private slots:
	void pageLoaded(bool ok);

signals:
	void loaded(const QUrl&);
private:
	QUrl url_;
	scoped_ptr<QWebPage> page_;
	QSharedMemory shared_memory_;
};

#endif
