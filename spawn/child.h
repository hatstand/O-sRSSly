#ifndef CHILD_H
#define CHILD_H

#include "spawn/spawnevent.pb.h"

#include <QObject>
#include <QQueue>
#include <QBuffer>
#include <QImage>
#include <QWebPage>
#include <QUrl>

#include <google/protobuf/message.h>

class QProcess;
class QSharedMemory;
class QPainter;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;

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
	
	void setUrl(const QUrl& url);
	void setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy);
	void setHtml(const QString& html);
	
	void paint(QPainter& p, const QRect& rect);

signals:
	void stateChanged(Child::State state);
	
	void repaintRequested(const QRect& rect);
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void statusBarMessage(const QString& text);
	void titleChanged(const QString& title);
	void urlChanged(const QUrl& url);
	void linkClicked(const QUrl& url);

private:
	// To be used by Manager
	Child(Manager* manager, quint64 id);
	void setReady();
	void setError();
	void setLastUrl(const QUrl& url);
	void restoreState();
	
	void clearQueue();
	QBuffer message_queue_;
	
	// For internal use
	bool resizeSharedMemory(int width, int height);
	void setLastHtml(const QString& url);
	
	Manager* manager_;
	quint64 id_;
	State state_;
	QSharedMemory* memory_;
	QImage image_;
	QUrl last_url_;
	QString last_html_;
};

}

#endif

