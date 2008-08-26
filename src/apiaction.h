#ifndef APIACTION_H
#define APIACTION_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class ApiAction : public QObject {
	Q_OBJECT
public:
	ApiAction(const QNetworkRequest& req,
		QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation,
		const QByteArray& content = QByteArray());

	~ApiAction();

	const QNetworkRequest& request() { return request_; }
	QNetworkReply* reply() { return reply_; }

	void start(QNetworkAccessManager* manager);

	void addToken(const QByteArray& t);
private:
	// Request this object looks after.
	QNetworkRequest request_;
	// Reply object given when request started.
	QNetworkReply* reply_;
	// GET/POST/HEAD etc.
	QNetworkAccessManager::Operation op_;
	// POST content
	QByteArray content_;

private slots:
	void requestFinished();

signals:
	void started();
	void completed();
	void failed();
};


#endif
