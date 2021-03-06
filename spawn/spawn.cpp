#include "spawn.h"
#include "spawnevent.pb.h"
#include "spawnreply.pb.h"
#include "iodevicestream.h"
#include "qthelpers.h"
#include "page.h"

#include <QLocalSocket>
#include <QtDebug>
#include <QCoreApplication>
#include <QUrl>
#include <QApplication>
#include <QPalette>

namespace Spawn {

Spawn::Spawn(const QString& server, QObject* parent)
	: QObject(parent),
	  socket_(new QLocalSocket(this)),
	  input_stream_(new google::protobuf::io::CopyingInputStreamAdaptor(new IODeviceInputStream(socket_))),
	  coded_input_stream_(new google::protobuf::io::CodedInputStream(input_stream_))
{
	input_stream_->SetOwnsCopyingStream(true);
	
	qDebug() << __PRETTY_FUNCTION__;
	qDebug() << "Connecting to" << server;
	
	connect(socket_, SIGNAL(readyRead()), SLOT(socketReadyRead()));
	connect(socket_, SIGNAL(disconnected()), SLOT(socketDisconnected()));
	
	socket_->connectToServer(server);
	
	// Mess with the application palette to make sure QWebPage always
	// paints its background white
	QPalette palette(QApplication::palette());
	palette.setColor(QPalette::Window, Qt::white);
	QApplication::setPalette(palette);
}

Spawn::~Spawn() {
	qDebug() << __PRETTY_FUNCTION__;
	delete input_stream_;
	delete coded_input_stream_;
	qDeleteAll(pages_);
}

void Spawn::socketReadyRead() {
	//qDebug() << __PRETTY_FUNCTION__;
	
	int bytesRemaining = socket_->bytesAvailable();
	
	while (bytesRemaining > 0) {
		quint32 size;
		SpawnEvent m;
		
		coded_input_stream_->ReadVarint32(&size);
		google::protobuf::io::CodedInputStream::Limit limit = coded_input_stream_->PushLimit(size);
		m.ParseFromCodedStream(coded_input_stream_);
		coded_input_stream_->PopLimit(limit);
		
		//qDebug() << "Read message" << m;
		processEvent(m);
		
		bytesRemaining -= size + google::protobuf::io::CodedOutputStream::VarintSize32(size);
	}
}

void Spawn::processEvent(const SpawnEvent& m) {
	//qDebug() << __PRETTY_FUNCTION__;
	
	quint64 id = m.destination();
	
	switch (m.type()) {
	case SpawnEvent_Type_NEW_PAGE:
		pages_[id] = new Page(id);
		connect(pages_[id], SIGNAL(reply(const SpawnReply&)), SLOT(sendReply(const SpawnReply&)));
		break;
	
	case SpawnEvent_Type_CLOSE:
		if (m.has_destination())
			delete pages_.take(id);
		else
			QCoreApplication::quit();
		break;
	
	case SpawnEvent_Type_RESIZE_EVENT:
		pages_[id]->resize(m.resize_event().width(), m.resize_event().height(), QString::fromStdString(m.resize_event().memory_key()));
		break;
	
	case SpawnEvent_Type_MOUSE_MOVE_EVENT:
	case SpawnEvent_Type_MOUSE_PRESS_EVENT:
	case SpawnEvent_Type_MOUSE_RELEASE_EVENT:
		pages_[id]->mouseEvent(m.type(), m.mouse_event());
		break;
	
	case SpawnEvent_Type_WHEEL_EVENT:
		pages_[id]->wheelEvent(m.mouse_event());
		break;
	
	case SpawnEvent_Type_KEY_PRESS_EVENT:
	case SpawnEvent_Type_KEY_RELEASE_EVENT:
		pages_[id]->keyEvent(m.type(), m.key_event());
		break;
	
	case SpawnEvent_Type_SET_URL:
		pages_[id]->setUrl(QUrl(QString::fromStdString(m.simple_string())));
		break;
	
	case SpawnEvent_Type_SET_LINK_DELEGATION_POLICY:
		pages_[id]->setLinkDelegationPolicy(m.simple_int());
		break;
	
	case SpawnEvent_Type_SET_HTML:
		pages_[id]->setHtml(QString::fromStdString(m.simple_string()));
		break;
		
	default:
		qDebug() << "Unhandled message" << m;
		break;
	}
}

void Spawn::sendReply(const SpawnReply& m) {
	int messageSize = m.ByteSize();
	//qDebug() << "Sending reply" << m << messageSize;
	
	// TODO: These don't seem to work when they're members.  Find out why.
	IODeviceOutputStream stream(socket_);
	google::protobuf::io::CopyingOutputStreamAdaptor zeroCopyStream(&stream);
	google::protobuf::io::CodedOutputStream codedStream(&zeroCopyStream);
	codedStream.WriteVarint32(messageSize);
	m.SerializeWithCachedSizes(&codedStream);
}

void Spawn::socketDisconnected() {
	QCoreApplication::quit();
}

}
