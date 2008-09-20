#ifndef SPAWN_H
#define SPAWN_H

#include <QObject>
#include <QMap>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

class QLocalSocket;
class QWebPage;
class QSharedMemory;

class SpawnEvent;

namespace Spawn {

class Page {
public:
	Page(QWebPage* p = NULL) : page(p), memory(NULL) {}
	~Page();
	
	QWebPage* page;
	QSharedMemory* memory;
};

class Spawn : public QObject {
	Q_OBJECT
public:
	Spawn(const QString& server, QObject* parent = 0);
	~Spawn();

private slots:
	void socketReadyRead();

private:
	void processEvent(const SpawnEvent& m);
	void newPage(quint64 id);
	void closePage(quint64 id);
	void sharedMemoryChanged(quint64 id, const QString& key);
	
	QLocalSocket* socket_;
	google::protobuf::io::CopyingInputStreamAdaptor* stream_;
	google::protobuf::io::CodedInputStream* coded_stream_;
	
	QMap<quint64, Page*> pages_;
};

}

#endif
