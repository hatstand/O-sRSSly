#include "readerapi.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QStringList>

const char* ReaderApi::kApplicationSource = "PurpleHatstands-Feeder-1";
const char* ReaderApi::kServiceName = "reader";
const char* ReaderApi::kAccountType = "HOSTED_OR_GOOGLE";

ReaderApi::ReaderApi(const QString& username, const QString& password, QObject* parent) 
	: QObject(parent), network_(new QNetworkAccessManager(this)) {
	QNetworkRequest login_request(QUrl("https://www.google.com/accounts/ClientLogin"));
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

	getSubscriptionList();
}

bool ReaderApi::isLoggedIn() {
	return !auth_.isEmpty();
}

void ReaderApi::getSubscriptionList() {
	qDebug() << __PRETTY_FUNCTION__;
	
	QNetworkRequest req(QUrl("http://www.google.com/reader/api/0/subscription/list"));
	req.setRawHeader(QByteArray("Authorization"), ("GoogleLogin " + auth_).toUtf8());

	QNetworkReply* reply = network_->get(req);

	connect(reply, SIGNAL(finished()), SLOT(getSubscriptionListComplete()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(networkError(QNetworkReply::NetworkError)));
}

void ReaderApi::getSubscriptionListComplete() {
	qDebug() << __PRETTY_FUNCTION__;
	qDebug() << static_cast<QNetworkReply*>(sender())->readAll();
}

void ReaderApi::networkError(QNetworkReply::NetworkError code) {
	qDebug() << __PRETTY_FUNCTION__ << code;
}

void ReaderApi::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
	qDebug() << __PRETTY_FUNCTION__;
}
