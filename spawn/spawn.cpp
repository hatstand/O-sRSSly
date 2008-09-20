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

Page::Page(QWebPage* p)
	: page_(p),
	  memory_(NULL)
{
}

Page::~Page() {
	delete page_;
	delete memory_;
}

void Page::resize(int width, int height, const QString& memoryKey) {
	delete memory_;
	memory_ = new QSharedMemory(memoryKey);
	memory_->attach();
	
	page_->setViewportSize(QSize(width, height));
	image_ = QImage(reinterpret_cast<uchar*>(memory_->data()), width, height, QImage::Format_RGB32);
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
	
	quint64 id = m.destination();
	
	switch (m.type()) {
	case SpawnEvent_Type_NEW_PAGE_EVENT:
		pages_[id] = new Page(new QWebPage(this));
		break;
	
	case SpawnEvent_Type_CLOSE_EVENT:
		if (m.has_destination())
			delete pages_.take(id);
		else
			QCoreApplication::quit();
		break;
	
	case SpawnEvent_Type_RESIZE_EVENT:
		pages_[id]->resize(m.resize_event().width(), m.resize_event().height(), QString::fromStdString(m.resize_event().memory_key()));
		break;
		
	default:
		qDebug() << "Unhandled message" << m;
		break;
	}
}

}
