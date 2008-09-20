#include "spawn.h"
#include "spawnevent.pb.h"
#include "iodevicestream.h"
#include "qthelpers.h"

#include <QLocalSocket>
#include <QtDebug>

namespace Spawn {

Spawn::Spawn(const QString& server, QObject* parent)
	: QObject(parent),
	  socket_(new QLocalSocket(this)),
	  stream_(new google::protobuf::io::CopyingInputStreamAdaptor(new IODeviceInputStream(socket_)))
{
	stream_->SetOwnsCopyingStream(true);
	
	qDebug() << __PRETTY_FUNCTION__;
	qDebug() << "Connecting to" << server;
	
	connect(socket_, SIGNAL(readyRead()), SLOT(socketReadyRead()));
	
	socket_->connectToServer(server);
}

Spawn::~Spawn() {
	delete stream_;
}

void Spawn::socketReadyRead() {
	qDebug() << __PRETTY_FUNCTION__;
	SpawnEvent m;
	m.ParseFromZeroCopyStream(stream_);
	
	qDebug() << m;
}

}
