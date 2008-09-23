#ifndef PAGE_H
#define PAGE_H

#include "spawnevent.pb.h"

#include <QObject>
#include <QImage>

class QWebPage;
class QSharedMemory;
class QRect;
class QUrl;

namespace Spawn {

class SpawnReply;

class Page : public QObject {
	Q_OBJECT
public:
	Page(quint64 id);
	~Page();
	
	quint64 id() const { return id_; }
	
	void resize(int width, int height, const QString& memoryKey);
	void mouseEvent(SpawnEvent_Type type, const MouseEvent& e);
	void keyEvent(SpawnEvent_Type type, const KeyEvent& e);
	void wheelEvent(const MouseEvent& e);
	
	void setUrl(const QUrl& url);
	void setLinkDelegationPolicy(int policy);
	void setHtml(const QString& html);

signals:
	void reply(const SpawnReply& reply);

private slots:
	void repaintRequested(const QRect& dirtyRect = QRect());
	void loadFinished(bool ok);
	void loadProgress(int progress);
	void loadStarted();
	void statusBarMessage(const QString& text);
	void titleChanged(const QString& title);
	void urlChanged(const QUrl& url);
	void linkClicked(const QUrl& url);
	void scrollRequested(int dx, int dy, const QRect& rectToScroll);

private:
	quint64 id_;
	QSharedMemory* memory_;
	QWebPage* page_;
	QImage image_;
	bool no_recursion_please_;
};

}

#endif
