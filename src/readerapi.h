#ifndef READERAPI_H
#define READERAPI_H

#include <QNetworkReply>
#include <QObject>

class QNetworkAccessManager;

class ReaderApi : public QObject {
Q_OBJECT
public:
	ReaderApi(const QString& username, const QString& password, QObject* parent = 0);
	virtual ~ReaderApi();

	bool isLoggedIn();
	void getSubscriptionList();

private slots:
	void loginComplete();
	void getSubscriptionListComplete();
	void networkError(QNetworkReply::NetworkError code);
	void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
	QNetworkAccessManager* network_;

	// Needed for editing things on Reader
	QString auth_;
	// Used in the cookie to show who we are
	QString sid_;

	static const char* kApplicationSource;
	static const char* kServiceName;
	static const char* kAccountType;
};

#endif
