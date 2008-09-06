#ifndef BROWSER_H
#define BROWSER_H

#include "ui_browser.h"

class Browser : public QWidget {
	Q_OBJECT
public:
	Browser(QWidget* parent = 0);
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
	Ui_Browser ui_;
};

#endif
