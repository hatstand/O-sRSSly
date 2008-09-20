#include "manager.h"
#include "child.h"
#include "qthelpers.h"
#include "iodevicestream.h"

#include "spawnevent.pb.h"
#include "newpageevent.pb.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <iostream>

namespace Spawn {

const char* const Manager::kSpawnArgument = "--spawn";

QString Manager::serverName() {
	QStringList args(QCoreApplication::arguments());
	for (int i=1 ; i<args.count() ; ++i) {
		if (args[i] == kSpawnArgument) {
			if (i+1 >= args.count()) {
				qFatal("Server name required after %s\n", kSpawnArgument);
			}
			
			return args[i+1];
		}
	}
	
	return QString::null;
}

Manager::Manager(QObject* parent, const QString& executable)
	: QObject(parent),
	  server_(new QLocalServer(this)),
	  next_child_id_(0),
	  executable_(executable)
{
	if (executable_.isNull()) {
		executable_ = QCoreApplication::arguments()[0];
	}
	
	connect(server_, SIGNAL(newConnection()), SLOT(newConnection()));
	while (true) {
		QString name = "feeder-" + QString::number(QCoreApplication::applicationPid()) + "-" + QString::number(qrand());
		if (server_->listen(name)) {
			qDebug() << "Listening on" << name;
			break;
		}
	}
}

Manager::~Manager() {
}

Child* Manager::createPage() {
	qDebug() << __PRETTY_FUNCTION__;
	// For now each page has its own process, but later we might want
	// to have some pages sharing - in that case just send a NewPage
	// message to an existing process
	
	// Make the child
	Child* child = new Child(this, next_child_id_++);
	children_waiting_for_socket_.enqueue(child);
	
	// Make the process
	QProcess* process = new QProcess(this);
	process->setProcessChannelMode(QProcess::ForwardedChannels);
	//connect(process, SIGNAL(started()), SLOT(processStarted()));
	connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	process->start(executable_, QStringList() << kSpawnArgument << server_->serverName());
	
	// Queue a message to create a new page
	NewPageEvent newPage;
	newPage.set_id(child->id());
	
	SpawnEvent message;
	message.set_type(SpawnEvent_Type_NEW_PAGE_EVENT);
	*(message.mutable_new_page_event()) = newPage;
	
	sendMessage(child, message);
	
	// Return the child
	return child;
}

void Manager::destroyPage(Child* child) {
	qDebug() << __PRETTY_FUNCTION__;
	if (children_.contains(child)) {
		// If this child had a socket open, close it
		// TODO
	} else {
		// Otherwise it must've been waiting
		children_waiting_for_socket_.removeAll(child);
	}
}

void Manager::newConnection() {
	qDebug() << __PRETTY_FUNCTION__;
	QLocalSocket* socket = server_->nextPendingConnection();
	sockets_ << socket;
	
	if (children_waiting_for_socket_.isEmpty()) {
		return;
	}
	
	// Get the next child that's waiting for a new socket
	Child* child = children_waiting_for_socket_.dequeue();
	
	// Assign it the socket
	children_[child] = socket;
	child->setReady();
	
	// Send any messages that were queued
	if (child->message_queue_.size() != 0) {
		*socket << child->message_queue_;
	}
}

void Manager::processError(QProcess::ProcessError error) {
	qDebug() << __PRETTY_FUNCTION__;
	switch (error) {
	case QProcess::FailedToStart:
		qDebug() << "Process failed to start";
		break;
	default:
		qDebug() << "Process error" << error;
		break;
	}
}


void Manager::sendMessage(Child* child, const google::protobuf::Message& m) {
	if (!child->isReady()) {
		m.AppendToString(&child->message_queue_);
		return;
	}
	
	QLocalSocket* socket = children_[child];
	
	IODeviceOutputStream stream(socket);
	google::protobuf::io::CopyingOutputStreamAdaptor zeroCopyStream(&stream);
	m.SerializeToZeroCopyStream(&zeroCopyStream);
}



}
