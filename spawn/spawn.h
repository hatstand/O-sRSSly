#ifndef SPAWN_H
#define SPAWN_H

#include <QObject>

#include <google/protobuf/io/zero_copy_stream_impl.h>

class QLocalSocket;

namespace Spawn {

class Spawn : public QObject {
	Q_OBJECT
public:
	Spawn(const QString& server, QObject* parent = 0);
	~Spawn();

private slots:
	void socketReadyRead();

private:
	QLocalSocket* socket_;
	google::protobuf::io::CopyingInputStreamAdaptor* stream_;
};

}

#endif
