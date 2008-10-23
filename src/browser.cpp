#include "browser.h"

#include <spawn/manager.h>
#include <spawn/view.h>

#include <QWebHistory>
#include <QWebView>
#include <QtDebug>

#ifdef USE_SPAWN
Browser::Browser(Spawn::Manager* manager, QWidget* parent)
#else
Browser::Browser(QWidget* parent)
#endif
	: QWidget(parent)
{
	ui_.setupUi(this);

#ifdef USE_SPAWN
	contents_ = new Spawn::View(manager, this);
#else
	contents_ = new QWebView(this);
#endif
	layout()->removeWidget(ui_.view_container_);
	delete ui_.view_container_;
	layout()->addWidget(contents_);
	
	connect(contents_, SIGNAL(loadStarted()), SLOT(loadStarted()));
	connect(contents_, SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
	connect(contents_, SIGNAL(urlChanged(const QUrl&)), SLOT(urlChanged(const QUrl&)));
	
	connect(ui_.address_, SIGNAL(returnPressed()), SLOT(returnPressed()));
	
	connect(contents_, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
	connect(contents_, SIGNAL(statusBarMessage(const QString&)), SIGNAL(statusBarMessage(const QString&)));
	connect(contents_, SIGNAL(iconChanged()), SIGNAL(iconChanged()));
	connect(contents_, SIGNAL(loadProgress(int)), SIGNAL(loadProgress(int)));
}

void Browser::loadStarted() {
	ui_.stack_->setCurrentIndex(1);
	
	//ui_.back_->setEnabled(contents_->history()->canGoBack());
	//ui_.forward_->setEnabled(contents_->history()->canGoForward());
}

void Browser::loadFinished() {
	ui_.stack_->setCurrentIndex(0);
}

void Browser::urlChanged(const QUrl& url) {
	ui_.address_->setText(url.toString());
}

void Browser::returnPressed() {
	QString url(ui_.address_->text());
	if (!url.contains("://"))
		url = "http://" + url;
	
	setUrl(QUrl(url, QUrl::TolerantMode));
}

void Browser::setUrl(const QUrl& url) {
	urlChanged(url);
	contents_->setUrl(url);
}

QIcon Browser::icon() const {
	return contents_->icon();
}
