#ifndef CHILD_H
#define CHILD_H

#include <QObject>
#include <QQueue>
#include <QBuffer>

#include <google/protobuf/message.h>

class QProcess;
class QSharedMemory;
class QPainter;

class MouseEvent;
class ResizeEvent;

namespace Spawn {

class Manager;

class Child : public QObject {
	Q_OBJECT
	friend class Manager;
public:
	enum State {
		Starting,
		Ready,
		Error
	};
	
	quint64 id() const { return id_; }
	State state() const { return state_; }
	bool isReady() const { return state_ == Ready; }
	bool isError() const { return state_ == Error; }
	
	void sendMouseEvent(const MouseEvent& e);
	void sendResizeEvent(const ResizeEvent& e);
	
	void paint(QPainter& p);

signals:
	void ready();
	void error();

private:
	// To be used by Manager
	Child(Manager* manager, quint64 id);
	void setReady();
	void setError();
	
	void clearQueue();
	QBuffer message_queue_;
	
	// For internal use
	void resizeSharedMemory(int width, int height);
	
	Manager* manager_;
	quint64 id_;
	State state_;
	QSharedMemory* memory_;
};

}

#endif

