#include "readerapi.h"
#include "subscriptionlist.h"
#include "xmlutils.h"

#include <string>

#include <QBuffer>
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

const char* ReaderApi::kReadingList("user/-/state/com.google/reading-list");

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

// Search feeds url
const QUrl ReaderApi::kSearchUrl("http://www.google.com/reader/api/0/search/items/ids");
const QUrl ReaderApi::kIdConvertUrl("http://www.google.com/reader/api/0/stream/items/contents");


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
	watchReply(login);

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

	connect(&throttle_clear_, SIGNAL(timeout()), SLOT(clearThrottle()));
	throttle_clear_.start(5*60*1000);

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
	watchReply(reply);

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
	watchReply(reply);

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
	watchReply(reply);
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
	setState(e, kReadTag, true);
}

void ReaderApi::setStarred(const AtomEntry& e, bool starred) {
	setState(e, kStarredTag, starred);
}

void ReaderApi::setState(const AtomEntry& e, const char* state, bool set) {
	qDebug() << __PRETTY_FUNCTION__ << state;

	QString content;
	content.sprintf("i=%s&%s=%s&ac=edit", e.id.toStdString().c_str(), (set ? "a" : "r"), state);

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
	QUrl url(encodeFeedId(s.id()));
	if (!continuation.isEmpty())
		url.addQueryItem("c", continuation);

	getSubscription(url);
}

void ReaderApi::getSubscription(const QString& id, const QString& continuation) {
	QUrl url(encodeFeedId(id));

	if (!continuation.isEmpty())
		url.addQueryItem("c", continuation);

	getSubscription(url);
}

QUrl ReaderApi::encodeFeedId(const QString& id) {
	// Hackery to percent encode the id.
	if (!id.startsWith("feed/") && !id.startsWith("user/"))
		return QUrl();
	
	QString real_id = id;
	// Don't encode feed or user.
	QString type = real_id.left(5);
	real_id.remove(0, 5);

	QString encoded_url = kAtomUrl.toString() + type + QUrl::toPercentEncoding(real_id);

	QUrl url;
	url.setEncodedUrl(encoded_url.toAscii(), QUrl::StrictMode);

	return url;
}

void ReaderApi::getSubscription(const QString& id, int count, const QString& timestamp) {
	QUrl url(encodeFeedId(id));
	// Fetch `count' items from timestamp forwards.
	url.addQueryItem("n", QString::number(count));
	url.addQueryItem("r", "o");
	url.addQueryItem("ot", timestamp);

	getSubscription(url);
}

void ReaderApi::getSubscription(const QUrl& url) {
	qDebug() << __PRETTY_FUNCTION__ << url;
	QNetworkRequest req(url);

	if (!checkThrottle(url))
		return;

	QNetworkReply* reply = network_->get(req);
	watchReply(reply);
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
	QUrl url(kAtomUrl.toString() + kReadingList);
	url.addQueryItem("client", kApplicationSource);
	url.addQueryItem("n", "100");
	url.addQueryItem("xt", kReadTag);

	if (!checkThrottle(url))
		return;

	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	watchReply(reply);
	connect(reply, SIGNAL(finished()), SLOT(getFreshComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getFreshComplete() {
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	AtomFeed feed(reply->url(), reply);
	continuation_ = feed.continuation();
	emit freshArrived(feed);

	reply->deleteLater();
}

void ReaderApi::getCategory(const QString& category, const QString& continuation) {
	QUrl url(kAtomUrl.toString() + category);
	if (!continuation.isEmpty())
		url.addQueryItem("c", continuation);

	if (!checkThrottle(url))
		return;

	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	watchReply(reply);
	connect(reply, SIGNAL(finished()), SLOT(getCategoryComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),	
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getCategoryComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

	AtomFeed feed(reply->url(), reply);
	emit categoryArrived(feed);

	qDebug() << reply->readAll();

	reply->deleteLater();
}

QMap<QString, QPair<int, QString> > ReaderApi::parseUnreadCounts(QIODevice* device) {
	qDebug() << __PRETTY_FUNCTION__;

	QXmlStreamReader s(device);

	QMap<QString, QPair<int, QString> > unread_counts;

	bool inside_list = false;
	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "list")
					inside_list = true;
				else if (inside_list && s.name() == "object")
					parseFeedUnreadCount(s, &unread_counts);
				else if (s.name() != "object")
					XmlUtils::ignoreElement(s);

				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "list")
					return unread_counts;

				break;

			default:
				break;
		}
	}

	return unread_counts;
}

void ReaderApi::parseFeedUnreadCount(QXmlStreamReader& s, QMap<QString, QPair<int, QString> >* unread_counts) {
	QString id;
	int count = 0;
	QString newest_timestamp;

	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "string" && s.attributes().value("name") == "id")
					id = s.readElementText();
				else if (s.name() == "number" && s.attributes().value("name") == "count")
					count = s.readElementText().toInt();
				else if (s.name() == "number" && s.attributes().value("name") == "newestItemTimestampUsec") {
					newest_timestamp = s.readElementText();
					if (newest_timestamp.size() > 6)
						newest_timestamp.chop(6); // Chop off usec & msec
				}
				else
					XmlUtils::ignoreElement(s);

				break;

			case QXmlStreamReader::EndElement:
				if (s.name() == "object") {
					unread_counts->insert(id, qMakePair(count, newest_timestamp));
					return;
				}

				break;

			default:
				break;
		}
	}
}

void ReaderApi::watchReply(QNetworkReply* reply) {
	connect(reply, SIGNAL(finished()), SLOT(replyFinished()));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(replyDownloadProgress(qint64, qint64)));
	reply_progress_[reply] = 0;
	updateProgress();
}

void ReaderApi::replyDownloadProgress(qint64 progress, qint64 total) {
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply)
		return;
	
	int value = 0;
	if (total != -1)
		value = int(float(progress * 100) / total);
	reply_progress_[reply] = value;
	updateProgress();
}

void ReaderApi::replyFinished() {
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply)
		return;
	
	reply_progress_[reply] = 100;
	updateProgress();
}

void ReaderApi::updateProgress() {
	int total = 0;
	int value = 0;
	
	foreach (int v, reply_progress_.values()) {
		total += 100;
		value += v;
	}
	
	if (total == value)
		reply_progress_.clear();
	
	emit progressChanged(value, total);
}

bool ReaderApi::checkThrottle(const QUrl& url) {
	QMap<QUrl, QTime>::const_iterator it = network_throttle_.find(url);

	if (it == network_throttle_.end() || it.value().elapsed() > 5000) {
		// Url either not visited or not recently.
		QTime time;
		time.start();
		network_throttle_.insert(url, time);

		return true;
	}

	qDebug() << url << "throttled...";
	return false;
}

void ReaderApi::clearThrottle() {
	for (QMap<QUrl, QTime>::iterator it = network_throttle_.begin();
		it != network_throttle_.end(); ++it) {
		if (it.value().elapsed() > 60000) {
			network_throttle_.erase(it);
		}
	}
}

void ReaderApi::search(const QString& query) {
	QUrl url(kSearchUrl);
	url.addQueryItem("q", query);
	url.addQueryItem("num", QString::number(1000));
	url.addQueryItem("client", kApplicationSource);

	if (!checkThrottle(url))
		return;
	
	QNetworkRequest req(url);
	QNetworkReply* reply = network_->get(req);
	watchReply(reply);
	connect(reply, SIGNAL(finished()), SLOT(searchPart()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::searchPart() {
	qDebug() << __PRETTY_FUNCTION__;
	QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
	QStringList ids = parseIntermediateSearch(reply);

	reply->deleteLater();

	if (ids.isEmpty())
		return;

	QByteArray post_data;
	foreach (QString id, ids) {
		post_data += "i=";
		post_data += id;
		post_data += "&";
	}

	post_data.chop(1);

	QUrl url(kIdConvertUrl);
	url.addQueryItem("output", "xml");
	url.addQueryItem("client", kApplicationSource);

	QNetworkRequest req(url);
	ApiAction* action = new ApiAction(req, QNetworkAccessManager::PostOperation, post_data);
	connect(action, SIGNAL(completed()), SLOT(searchFinished()));

	queued_actions_.enqueue(action);
	processActionQueue();
}

QStringList ReaderApi::parseIntermediateSearch(QIODevice* data) {
	qDebug() << __PRETTY_FUNCTION__;
	QXmlStreamReader s(data);

	QStringList ids;

	while (!s.atEnd()) {
		QXmlStreamReader::TokenType type = s.readNext();
		switch (type) {
			case QXmlStreamReader::StartElement:
				if (s.name() == "number" && s.attributes().value("name") == "id")
					ids << s.readElementText();
				break;
			default:
				break;
		}
	}

	return ids;
}

void ReaderApi::searchFinished() {
	qDebug() << __PRETTY_FUNCTION__;
	ApiAction* action = static_cast<ApiAction*>(sender());

	std::string json(action->reply()->readAll().data());
	json::grammar<char>::variant v = json::parse(json.begin(), json.end());

	std::list<std::string> current_id;
	traverseJson(v, &current_id);
}

void ReaderApi::traverseJson(const json::grammar<char>::variant& var, std::list<std::string>* current_ids) {
	if (var->empty())
		return;

	if (var->type() == typeid(std::string)) {
		std::string value = boost::any_cast<std::string>(*var);
		qDebug() << current_ids->rbegin()->c_str() << ":" << value.c_str();
	} else if (var->type() == typeid(json::grammar<char>::array)) {
		const json::grammar<char>::array& a = boost::any_cast<json::grammar<char>::array>(*var);
		for (json::grammar<char>::array::const_iterator it = a.begin(); it != a.end(); ++it) {
			traverseJson(*it, current_ids);
		}
	} else if (var->type() == typeid(json::grammar<char>::object)) {
		const json::grammar<char>::object& o = boost::any_cast<json::grammar<char>::object>(*var);
		for (json::grammar<char>::object::const_iterator it = o.begin(); it != o.end(); ++it) {
			if (it->second->type() == typeid(std::string)) {
				current_ids->push_back(it->first);
				traverseJson(it->second, current_ids);
				current_ids->pop_back();
			}
		}
	}
}
