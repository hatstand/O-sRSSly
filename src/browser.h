#ifndef BROWSER_H
#define BROWSER_H

#include "ui_browser.h"

namespace Spawn {
	class Manager;
	class View;
}

class Browser : public QWidget {
	Q_OBJECT
public:
	Browser(Spawn::Manager* manager, QWidget* parent = 0);
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
	Spawn::View* contents_;
	Ui_Browser ui_;
};

#endif
