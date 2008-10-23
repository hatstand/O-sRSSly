#ifndef BROWSER_H
#define BROWSER_H

#include "ui_browser.h"

namespace Spawn {
	class Manager;
	class View;
}

class QWebView;

class Browser : public QWidget {
	Q_OBJECT
public:
#ifdef USE_SPAWN
	Browser(Spawn::Manager* manager, QWidget* parent = 0);
#else
	Browser(QWidget* parent = 0);
#endif
	virtual ~Browser() {}
	
	QIcon icon() const;

signals:
	void titleChanged(const QString& title);
	void statusBarMessage(const QString& message);
	void iconChanged();
	void loadProgress(int progress);

public slots:
	void setUrl(const QUrl& url);
	
private slots:
	void loadStarted();
	void loadFinished();
	void urlChanged(const QUrl& url);
	
	void returnPressed();
	
private:
#ifdef USE_SPAWN
	Spawn::View* contents_;
#else
	QWebView* contents_;
#endif
	Ui_Browser ui_;
};

#endif
