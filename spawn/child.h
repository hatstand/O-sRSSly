#ifndef CHILD_H
#define CHILD_H

#include <QObject>
#include <QQueue>

#include <google/protobuf/message.h>

class QProcess;

class MouseEvent;

namespace Spawn {

class Manager;

class Child : public QObject {
	Q_OBJECT
	friend class Manager;
public:
	quint64 id() const { return id_; }
	bool isReady() const { return ready_; }
	
	void sendMouseEvent(const MouseEvent& e);

signals:
	void ready();

private:
	// To be used by Manager
	Child(Manager* manager, quint64 id);
	void setReady();
	
	std::string message_queue_;
	
	// For internal use
	Manager* manager_;
	quint64 id_;
	bool ready_;
};

}

#endif

