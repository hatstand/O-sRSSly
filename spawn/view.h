#ifndef SPAWNVIEW_H
#define SPAWNVIEW_H

#include <QWidget>
#include <QUrl>

namespace Spawn {

class Manager;
class Child;

class View : public QWidget {
	Q_OBJECT
public:
	View(Manager* manager, QWidget* parent = 0);
	~View();
	
	void setUrl(const QUrl& url);

protected:
	bool event(QEvent* event);

private:
	int page_id_;
	Manager* manager_;
	Child* child_;
};

}

#endif
