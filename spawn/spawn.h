#ifndef SPAWN_H
#define SPAWN_H

#include <QObject>
#include <QMap>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

class QLocalSocket;

class SpawnEvent;
class SpawnReply;

namespace Spawn {

class Page;

class Spawn : public QObject {
	Q_OBJECT
public:
	Spawn(const QString& server, QObject* parent = 0);
	~Spawn();

private slots:
	void socketReadyRead();
	void sendReply(const SpawnReply& m);

private:
	void processEvent(const SpawnEvent& m);
	void sharedMemoryChanged(quint64 id, const QString& key);
	
	QLocalSocket* socket_;
	google::protobuf::io::CopyingInputStreamAdaptor* input_stream_;
	google::protobuf::io::CodedInputStream* coded_input_stream_;
	
	QMap<quint64, Page*> pages_;
};

}

#endif
