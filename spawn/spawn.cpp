#include "spawn.h"
#include "spawnevent.pb.h"
#include "iodevicestream.h"
#include "qthelpers.h"

#include <QLocalSocket>
#include <QtDebug>
#include <QCoreApplication>
#include <QWebPage>
#include <QSharedMemory>

namespace Spawn {

Page::~Page() {
	delete page;
	delete memory;
}

Spawn::Spawn(const QString& server, QObject* parent)
	: QObject(parent),
	  socket_(new QLocalSocket(this)),
	  stream_(new google::protobuf::io::CopyingInputStreamAdaptor(new IODeviceInputStream(socket_))),
	  coded_stream_(new google::protobuf::io::CodedInputStream(stream_))
{
	stream_->SetOwnsCopyingStream(true);
	
	qDebug() << __PRETTY_FUNCTION__;
	qDebug() << "Connecting to" << server;
	
	connect(socket_, SIGNAL(readyRead()), SLOT(socketReadyRead()));
	
	socket_->connectToServer(server);
}

Spawn::~Spawn() {
	qDebug() << __PRETTY_FUNCTION__;
	delete stream_;
	qDeleteAll(pages_);
}

void Spawn::socketReadyRead() {
	qDebug() << __PRETTY_FUNCTION__;
	
	int bytesRemaining = socket_->bytesAvailable();
	
	while (bytesRemaining > 0) {
		quint32 size;
		SpawnEvent m;
		
		coded_stream_->ReadVarint32(&size);
		google::protobuf::io::CodedInputStream::Limit limit = coded_stream_->PushLimit(size);
		m.ParseFromCodedStream(coded_stream_);
		coded_stream_->PopLimit(limit);
		
		qDebug() << "Read message" << m;
		processEvent(m);
		
		bytesRemaining -= size + google::protobuf::io::CodedOutputStream::VarintSize32(size);
	}
}

void Spawn::processEvent(const SpawnEvent& m) {
	qDebug() << __PRETTY_FUNCTION__;
	
	switch (m.type()) {
	case SpawnEvent_Type_NEW_PAGE_EVENT:
		newPage(m.destination());
		break;
	
	case SpawnEvent_Type_CLOSE_EVENT:
		if (m.has_destination())
			closePage(m.destination());
		else
			QCoreApplication::quit();
		break;
	
	case SpawnEvent_Type_SHARED_MEMORY_CHANGED:
		sharedMemoryChanged(m.destination(), QString::fromStdString(m.shared_memory_changed().key()));
		break;
		
	default:
		qDebug() << "Unhandled message" << m;
		break;
	}
}

void Spawn::newPage(quint64 id) {
	qDebug() << __PRETTY_FUNCTION__;
	
	pages_[id] = new Page(new QWebPage(this));
}

void Spawn::closePage(quint64 id) {
	qDebug() << __PRETTY_FUNCTION__;

	Page* page = pages_.take(id);
	delete page;
}

void Spawn::sharedMemoryChanged(quint64 id, const QString& key) {
	Page* page = pages_[id];
	delete page->memory;
	page->memory = new QSharedMemory(key, this);
	page->memory->attach();
}

}
