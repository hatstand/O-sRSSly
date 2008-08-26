#include "apiaction.h"

#include <QDebug>
#include <QRegExp>

ApiAction::ApiAction(const QNetworkRequest& req,
	QNetworkAccessManager::Operation op,
	const QByteArray& content)
	:	request_(req), reply_(0), op_(op), content_(content) {
}

ApiAction::~ApiAction() {
	delete reply_;
}

void ApiAction::start(QNetworkAccessManager* manager) {
	qDebug() << __PRETTY_FUNCTION__ << request_.url() << content_;
	switch (op_) {
		case QNetworkAccessManager::HeadOperation:
			reply_ = manager->head(request_);
			break;
		case QNetworkAccessManager::GetOperation:
			reply_ = manager->get(request_);
			break;
		case QNetworkAccessManager::PostOperation:
			reply_ = manager->post(request_, content_);
			break;
		case QNetworkAccessManager::PutOperation:
			reply_ = manager->put(request_, content_);
			break;
		default:
			qWarning() << "Something weird is going on";
			break;
	}

	Q_ASSERT(reply_);

	// Make sure we don't connect more than once.
	reply_->disconnect();
	connect(reply_, SIGNAL(finished()), SLOT(requestFinished()));
	emit started();
}

void ApiAction::requestFinished() {
	if (reply_->error() == QNetworkReply::NoError) {
		emit completed();
		deleteLater();
	} else {
		emit failed();
	}
}

void ApiAction::addToken(const QByteArray& t) {
	if (op_ == QNetworkAccessManager::PostOperation) {
		QString temp(content_);

		// Strip old token if present.
		if (temp.contains(QRegExp("&T=.{57}$"))) {
			temp.chop(60);
			content_ = temp.toUtf8();
		}

		content_ += "&T=";
		content_ += t;
	}
}
