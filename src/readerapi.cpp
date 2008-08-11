#include "readerapi.h"
#include "subscriptionlist.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QStringList>

// For Google ClientLogin service
const char* ReaderApi::kApplicationSource = "PurpleHatstands-Feeder-1";
const char* ReaderApi::kServiceName = "reader";
const char* ReaderApi::kAccountType = "HOSTED_OR_GOOGLE";

// ClientLogin url
const QUrl ReaderApi::kLoginUrl("https://www.google.com/accounts/ClientLogin");

// List API urls
const QUrl ReaderApi::kSubscriptionUrl("http://www.google.com/reader/api/0/subscription/list");
const QUrl ReaderApi::kTagsUrl("http://www.google.com/reader/api/0/tag/list");
const QUrl ReaderApi::kUnreadUrl("http://www.google.com/reader/api/0/unread-count");
const QUrl ReaderApi::kPrefsUrl("http://www.google.com/reader/api/0/preference/list");

// Url to get temporary authentication token.
const QUrl ReaderApi::kTokenUrl("http://www.google.com/reader/api/0/token");

// Edit Urls
const QUrl ReaderApi::kEditTagUrl("http://www.google.com/reader/api/0/subscription/edit-tag");


ReaderApi::ReaderApi(const QString& username, const QString& password, QObject* parent) 
	:	QObject(parent), network_(new QNetworkAccessManager(this)) {
	QNetworkRequest login_request(kLoginUrl);
	login_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QString content;
	content.sprintf("Email=%s&Passwd=%s&source=%s&service=%s&accountType=%s",
		username.toStdString().c_str(), password.toStdString().c_str(),
		kApplicationSource, kServiceName, kAccountType);

	qDebug() << "Logging in";
	QNetworkReply* login = network_->post(login_request, content.toUtf8());

	connect(login, SIGNAL(finished()), SLOT(loginComplete()));
	connect(login, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
	connect(network_, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
		SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));

	connect(this, SIGNAL(tokenReady()), SLOT(processActionQueue()));
}

ReaderApi::~ReaderApi() {
	
}

void ReaderApi::loginComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* login = static_cast<QNetworkReply*>(sender());
	QString data(login->readAll());

	QStringList list = data.split('\n');
	QMap<QString, QString> auth;
	foreach (QString s, list) {
		if (s.isEmpty())
			continue;

		QStringList l(s.split('='));
		auth.insert(l[0], l[1]);
	}

	auth_ = auth["Auth"];
	sid_ = auth["SID"];

	QList<QNetworkCookie> cookies;
	cookies << QNetworkCookie(QByteArray("SID"), sid_.toUtf8());
	network_->cookieJar()->setCookiesFromUrl(cookies, QUrl("http://www.google.com/"));
	qDebug() << network_->cookieJar()->cookiesForUrl(QUrl("http://www.google.com/"));

	getSubscriptionList();
}

bool ReaderApi::isLoggedIn() {
	return !auth_.isEmpty();
}

void ReaderApi::getSubscriptionList() {
	qDebug() << __PRETTY_FUNCTION__;
	
	QNetworkRequest req(kSubscriptionUrl);
	//req.setRawHeader(QByteArray("Authorization"), ("GoogleLogin " + auth_).toUtf8());

	QNetworkReply* reply = network_->get(req);

	connect(reply, SIGNAL(finished()), SLOT(getSubscriptionListComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getSubscriptionListComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	QByteArray xml(reply->readAll());

	QXmlStreamReader s(xml);
	SubscriptionList list(s);
	foreach (Subscription sub, list.subscriptions()) {
		qDebug() << sub;
	}

	delete reply;
}

void ReaderApi::getUnread() {
	qDebug() << __PRETTY_FUNCTION__;
	
	QNetworkRequest req(kUnreadUrl);

	QNetworkReply* reply = network_->get(req);

	connect(reply, SIGNAL(finished()), SLOT(getUnreadComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getUnreadComplete() {
	qDebug() << __PRETTY_FUNCTION__;

	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	qDebug() << reply->readAll();
	delete reply;
}

void ReaderApi::networkError(QNetworkReply::NetworkError code) {
	qDebug() << __PRETTY_FUNCTION__ << code;
}

void ReaderApi::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
	qDebug() << __PRETTY_FUNCTION__;
}

void ReaderApi::getToken() {
	qDebug() << __PRETTY_FUNCTION__;

	QNetworkRequest req(kTokenUrl);
	QNetworkReply* reply = network_->get(req);
	connect(reply, SIGNAL(finished()), SLOT(getTokenComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getTokenComplete() {
	qDebug() << __PRETTY_FUNCTION__;

	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	token_ = reply->readAll();

	processActionQueue();
	emit tokenReady();
	
	delete reply;
}

void ReaderApi::setRead(const AtomEntry& e) {
	qDebug() << __PRETTY_FUNCTION__;

	QNetworkRequest req(kEditTagUrl);
	ApiAction* action = new ApiAction(req, QNetworkAccessManager::PostOperation);

	queued_actions_.enqueue(action);

	processActionQueue();
}

void ReaderApi::processActionQueue() {
	if (!token_.isEmpty() && !getting_token_) {
		foreach (ApiAction* a, queued_actions_) {
			a->start(network_);
		}
	} else if (token_.isEmpty() && !getting_token_) {
		getToken();
	}
}
