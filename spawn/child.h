#ifndef CHILD_H
#define CHILD_H

#include <QObject>

class QProcess;

namespace Spawn {

class Manager;

class Child : public QObject {
	Q_OBJECT
	friend class Manager;
public:
	quint64 id() const { return id_; }
	bool isReady() const { return ready_; }

signals:
	void ready();

private:
	// To be used by Manager
	Child(Manager* manager, quint64 id);
	void setReady();
	
	QProcess* starting_process_;
	
	// For internal use
	Manager* manager_;
	quint64 id_;
	bool ready_;
};

}

#endif

