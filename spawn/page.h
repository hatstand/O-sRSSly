#ifndef PAGE_H
#define PAGE_H

#include <QObject>
#include <QImage>

class QWebPage;
class QSharedMemory;
class QRect;

class SpawnReply;

namespace Spawn {

class Page : public QObject {
	Q_OBJECT
public:
	Page(quint64 id);
	~Page();
	
	quint64 id() const { return id_; }
	
	void resize(int width, int height, const QString& memoryKey);

signals:
	void reply(const SpawnReply& reply);

private slots:
	void repaintRequested(const QRect& dirtyRect = QRect());

private:
	quint64 id_;
	QSharedMemory* memory_;
	QWebPage* page_;
	QImage image_;
	bool no_recursion_please_;
};

}

#endif
