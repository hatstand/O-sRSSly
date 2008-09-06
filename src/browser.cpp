#include "browser.h"

#include <QWebHistory>
#include <QtDebug>

Browser::Browser(QWidget* parent)
	: QWidget(parent)
{
	ui_.setupUi(this);
	
	connect(ui_.webView_, SIGNAL(loadStarted()), SLOT(loadStarted()));
	connect(ui_.webView_, SIGNAL(loadFinished(bool)), SLOT(loadFinished()));
	connect(ui_.webView_, SIGNAL(urlChanged(const QUrl&)), SLOT(urlChanged(const QUrl&)));
	
	connect(ui_.address_, SIGNAL(returnPressed()), SLOT(returnPressed()));
	
	connect(ui_.webView_, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
	connect(ui_.webView_, SIGNAL(statusBarMessage(const QString&)), SIGNAL(statusBarMessage(const QString&)));
	connect(ui_.webView_, SIGNAL(iconChanged()), SIGNAL(iconChanged()));
	connect(ui_.webView_, SIGNAL(loadProgress(int)), SIGNAL(loadProgress(int)));
}

void Browser::loadStarted() {
	ui_.stack_->setCurrentIndex(1);
	
	ui_.back_->setEnabled(ui_.webView_->history()->canGoBack());
	ui_.forward_->setEnabled(ui_.webView_->history()->canGoForward());
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
	ui_.webView_->setUrl(url);
}

QIcon Browser::icon() const {
	return ui_.webView_->icon();
}
