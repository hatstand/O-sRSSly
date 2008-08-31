#ifndef READERAPI_H
#define READERAPI_H

#include "apiaction.h"
#include "atomfeed.h"
#include "subscriptionlist.h"

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
	void login();
	void getSubscriptionList();
	void getSubscription(const Subscription& s, const QString& continuation = "");
	void getFresh();
	void getUnread();
	void getCategory(const QString& category, const QString& continuation = "");
	void setRead(const AtomEntry& e);
	void setStarred(const AtomEntry& e, bool starred);
	void addCategory(const Subscription& s, const QString& category);
	void removeCategory(const Subscription& s, const QString& category);

private:
	void getToken();
	void editCategory(const Subscription& s, const QString& category, bool add);
	void setState(const AtomEntry& e, const char* state, bool set);

private slots:
	void loginComplete();
	void getSubscriptionListComplete();
	void getSubscriptionComplete();
	void getFreshComplete();
	void getUnreadComplete();
	void getCategoryComplete();
	void getTokenComplete();
	void networkError(QNetworkReply::NetworkError code);
	void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
	void processActionQueue();
	void actionFailed();

signals:
	void loggedIn();
	void subscriptionListArrived(const SubscriptionList&);
	void subscriptionArrived(const AtomFeed&);
	void categoryArrived(const AtomFeed&);
	void freshArrived(const AtomFeed&);
	// Emitted when an auth token has been received.
	void tokenReady();

private:
	QNetworkAccessManager* network_;

	QString username_;
	QString password_;

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
	static const char* kReadTag;
	static const char* kStarredTag;
	static const char* kFreshTag;
	static const QUrl kEditSubscriptionUrl;

	static const QUrl kAtomUrl;
};

#endif
