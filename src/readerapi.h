#ifndef READERAPI_H
#define READERAPI_H

#include "apiaction.h"
#include "atomfeed.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>

class ReaderApi : public QObject {
Q_OBJECT
public:
	ReaderApi(const QString& username, const QString& password, QObject* parent = 0);
	virtual ~ReaderApi();

	bool isLoggedIn();
	void getSubscriptionList();
	void getUnread();
	void setRead(const AtomEntry& e);

private:
	void getToken();

private slots:
	void loginComplete();
	void getSubscriptionListComplete();
	void getUnreadComplete();
	void getTokenComplete();
	void networkError(QNetworkReply::NetworkError code);
	void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
	void processActionQueue();

signals:
	// Emitted when an auth token has been received.
	void tokenReady();

private:
	QNetworkAccessManager* network_;

	// Needed for editing things on Reader
	QString auth_;
	// Used in the cookie to show who we are
	QString sid_;

	// Temporary authentication token.
	QString token_;
	bool getting_token_;
	QQueue<ApiAction*> queued_actions_;


	static const char* kApplicationSource;
	static const char* kServiceName;
	static const char* kAccountType;

	static const QUrl kLoginUrl;

	static const QUrl kSubscriptionUrl;
	static const QUrl kTagsUrl;
	static const QUrl kUnreadUrl;
	static const QUrl kPrefsUrl;

	static const QUrl kTokenUrl;
	
	static const QUrl kEditTagUrl;
};

#endif
