#include "readerapi.h"
#include "subscriptionlist.h"

#include <QDebug>
#include <QFile>
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
const QUrl ReaderApi::kEditTagUrl("http://www.google.com/reader/api/0/edit-tag");
const char* ReaderApi::kReadTag("user/-/state/com.google/read");
const char* ReaderApi::kStarredTag("user/-/state/com.google/starred");
const char* ReaderApi::kFreshTag("user/-/state/com.google/fresh");
const QUrl ReaderApi::kEditSubscriptionUrl("http://www.google.com/reader/api/0/subscription/edit");

// Atom feed url base
const QUrl ReaderApi::kAtomUrl("http://www.google.com/reader/atom/");


ReaderApi::ReaderApi(const QString& username, const QString& password, QObject* parent) 
	:	QObject(parent), network_(new QNetworkAccessManager(this)),
		username_(username), password_(password), getting_token_(false) {

	// Catch SSL errors
	connect(network_, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
		SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));

	// When an auth token arrives, process the actions that were waiting for it.
	connect(this, SIGNAL(tokenReady()), SLOT(processActionQueue()));
}

ReaderApi::~ReaderApi() {
	
}

void ReaderApi::login() {
	QString content;
	content.sprintf("Email=%s&Passwd=%s&source=%s&service=%s&accountType=%s",
		username_.toStdString().c_str(), password_.toStdString().c_str(),
		kApplicationSource, kServiceName, kAccountType);

	qDebug() << "Logging in";
	QNetworkRequest login_request(kLoginUrl);
	login_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply* login = network_->post(login_request, content.toUtf8());

	connect(login, SIGNAL(finished()), SLOT(loginComplete()));
	connect(login, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::loginComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* login = static_cast<QNetworkReply*>(sender());
	QString data(login->readAll());

	QStringList list = data.split('\n');
	QMap<QString, QString> auth;
	foreach (const QString& s, list) {
		if (s.isEmpty())
			continue;

		QStringList l(s.split('='));
		auth.insert(l[0], l[1]);
	}

	auth_ = auth["Auth"];
	sid_ = auth["SID"];

	// Set the session id cookie
	QList<QNetworkCookie> cookies;
	cookies << QNetworkCookie(QByteArray("SID"), sid_.toUtf8());
	network_->cookieJar()->setCookiesFromUrl(cookies, QUrl("http://www.google.com/"));
	qDebug() << network_->cookieJar()->cookiesForUrl(QUrl("http://www.google.com/"));

	login->deleteLater();

	emit loggedIn();
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
	foreach (const Subscription& sub, list.subscriptions()) {
		qDebug() << sub;
	}

	reply->deleteLater();

	emit subscriptionListArrived(list);
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
	reply->deleteLater();
}

void ReaderApi::networkError(QNetworkReply::NetworkError code) {
	qDebug() << __PRETTY_FUNCTION__ << code;
}

void ReaderApi::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
	qDebug() << __PRETTY_FUNCTION__;
}

void ReaderApi::getToken() {
	qDebug() << __PRETTY_FUNCTION__;

	getting_token_ = true;

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

	getting_token_ = false;

	processActionQueue();
	emit tokenReady();
	
	reply->deleteLater();
}

void ReaderApi::setRead(const AtomEntry& e) {
	setState(e, kReadTag);
}

void ReaderApi::setStarred(const AtomEntry& e) {
	setState(e, kStarredTag);
}

void ReaderApi::setState(const AtomEntry& e, const char* state) {
	qDebug() << __PRETTY_FUNCTION__ << state;

	QString content;
	content.sprintf("i=%s&a=%s&ac=edit", e.id.toStdString().c_str(), state);

	QUrl url(kEditTagUrl);
	url.addQueryItem("client", kApplicationSource);
	QNetworkRequest req(url);
	ApiAction* action = new ApiAction(req, QNetworkAccessManager::PostOperation, content.toUtf8());

	queued_actions_.enqueue(action);

	processActionQueue();
}

void ReaderApi::addCategory(const Subscription& s, const QString& category) {
	qDebug() << __PRETTY_FUNCTION__;

	editCategory(s, category, true);
}

void ReaderApi::removeCategory(const Subscription& s, const QString& category) {
	qDebug() << __PRETTY_FUNCTION__;

	editCategory(s, category, false);
}

void ReaderApi::editCategory(const Subscription& s, const QString& category, bool add) {
	qDebug() << __PRETTY_FUNCTION__;

	const char* add_remove = (add ? "a" : "r");

	QString content;
	content.sprintf("s=%s&%s=%s&ac=edit",
		s.id().toStdString().c_str(), add_remove, category.toStdString().c_str());
	
	QUrl url(kEditSubscriptionUrl);
	url.addQueryItem("client", kApplicationSource);
	QNetworkRequest req(url);
	ApiAction* action = new ApiAction(req, QNetworkAccessManager::PostOperation, content.toUtf8());

	queued_actions_.enqueue(action);
	processActionQueue();
}

void ReaderApi::processActionQueue() {
	qDebug() << __PRETTY_FUNCTION__;
	if (!token_.isEmpty() && !getting_token_) {
		while (!queued_actions_.isEmpty()) {
			ApiAction* a = queued_actions_.dequeue();
			a->addToken(token_.toUtf8());
			connect(a, SIGNAL(failed()), SLOT(actionFailed()));
			a->start(network_);
		}
	} else if (token_.isEmpty() && !getting_token_) {
		getToken();
	}
}

void ReaderApi::getSubscription(const Subscription& s, const QString& continuation) {
	QUrl url(kAtomUrl.toString() + s.id());
	if (!continuation.isEmpty())
		url.addQueryItem("c", continuation);

	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	connect(reply, SIGNAL(finished()), SLOT(getSubscriptionComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getSubscriptionComplete() {
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	AtomFeed feed(reply->url(), reply);

	if (!feed.hasError())
		emit subscriptionArrived(feed);
	else {
		qWarning() << "Error parsing feed:" << reply->url();
	}

	reply->deleteLater();
}

void ReaderApi::actionFailed() {
	// Token probably expired.
	if (!getting_token_) {
		token_.clear();
		getToken();
	}

	queued_actions_.enqueue(static_cast<ApiAction*>(sender()));
}

void ReaderApi::getFresh() {
	QUrl url(kAtomUrl.toString() + kFreshTag);

	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	connect(reply, SIGNAL(finished()), SLOT(getFreshComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getFreshComplete() {
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	AtomFeed feed(reply->url(), reply);
	
	emit freshArrived(feed);

	reply->deleteLater();
}

void ReaderApi::getCategory(const QString& category, const QString& continuation) {
	QUrl url(kAtomUrl.toString() + category);
	if (!continuation.isEmpty())
		url.addQueryItem("c", continuation);

	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	connect(reply, SIGNAL(finished()), SLOT(getCategoryComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),	
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getCategoryComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	AtomFeed feed(reply->url(), reply);
	emit categoryArrived(feed);

	reply->deleteLater();
}

