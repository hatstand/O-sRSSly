#ifndef CHILD_H
#define CHILD_H

#include "spawnevent.pb.h"

#include <QObject>
#include <QQueue>
#include <QBuffer>
#include <QImage>

#include <google/protobuf/message.h>

class QProcess;
class QSharedMemory;
class QPainter;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QUrl;

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
	
	void sendMouseEvent(SpawnEvent_Type type, QMouseEvent* event);
	void sendKeyEvent(SpawnEvent_Type type, QKeyEvent* event);
	void sendWheelEvent(QWheelEvent* event);
	void sendResizeEvent(int width, int height);
	
	void paint(QPainter& p, const QRect& rect);

signals:
	void ready();
	void error();
	
	void repaintRequested(const QRect& rect);
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void statusBarMessage(const QString& text);
	void titleChanged(const QString& title);
	void urlChanged(const QUrl& url);

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
	QImage image_;
};

}

#endif

