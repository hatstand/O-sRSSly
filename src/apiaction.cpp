#include "apiaction.h"

#include <QDebug>

ApiAction::ApiAction(const QNetworkRequest& req,
	QNetworkAccessManager::Operation op,
	const QByteArray& content)
	:	request_(req), reply_(0), op_(op), content_(content) {
}

ApiAction::~ApiAction() {
	delete reply_;
}

void ApiAction::start(QNetworkAccessManager* manager) {
	qDebug() << __PRETTY_FUNCTION__ << request_.url();
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

	connect(reply_, SIGNAL(finished()), SLOT(requestFinished()));
	emit started();
}

void ApiAction::requestFinished() {
	qDebug() << __PRETTY_FUNCTION__ << reply_->error();
	emit completed();
}

void ApiAction::addToken(const QByteArray& t) {
	if (op_ == QNetworkAccessManager::PostOperation) {
		content_ += "&T=";
		content_ += t;
	}
	qDebug() << __PRETTY_FUNCTION__ << content_;
}
