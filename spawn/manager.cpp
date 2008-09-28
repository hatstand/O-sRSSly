#include "manager.h"
#include "child.h"
#include "qthelpers.h"
#include "iodevicestream.h"

#include "spawnevent.pb.h"
#include "controlevents.pb.h"
#include "spawnreply.pb.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUrl>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

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
	qDebug() << __PRETTY_FUNCTION__;
	
	// Destroy all pages
	// This will release shared memory and terminate the processes
	foreach (Child* child, children_.keys()) {
		destroyPage(child);
	}
	
	// Since we're not going back to the event loop, make sure all the
	// sockets write their data
	foreach (QLocalSocket* socket, sockets_) {
		while (socket->bytesToWrite() > 0) {
			qDebug() << "Flushing data to" << socket;
			socket->waitForBytesWritten(1000);
		}
	}
	
	// Now wait for all child processes to shutdown
	// Kill them if we need to
	foreach (QProcess* process, processes_) {
		process->waitForFinished(1000);
		if (process->state() != QProcess::Running) continue;
		
		qDebug() << "Process" << process << "still running - sending SIGTERM";
		process->terminate();
		process->waitForFinished(1000);
		if (process->state() != QProcess::Running) continue;
		
		qDebug() << "Process" << process << "still running - sending SIGKILL";
		process->kill();
	}
}

Child* Manager::createPage() {
	qDebug() << __PRETTY_FUNCTION__;
	// For now each page has its own process, but later we might want
	// to have some pages sharing - in that case just send a NewPage
	// message to an existing process
	
	// Make the child
	Child* child = new Child(this, next_child_id_++);
	children_waiting_for_socket_.enqueue(child);
	pages_[child->id()] = child;
	
	// Make the process
	QProcess* process = new QProcess(this);
	process->setProcessChannelMode(QProcess::ForwardedChannels);
	//connect(process, SIGNAL(started()), SLOT(processStarted()));
	connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished()));
	
	process->start(executable_, QStringList() << kSpawnArgument << server_->serverName());
	processes_ << process;
	
	// Queue a message to create a new page
	NewPage newPage;
	
	SpawnEvent message;
	message.set_type(SpawnEvent_Type_NEW_PAGE);
	message.set_destination(child->id());
	*(message.mutable_new_page()) = newPage;
	
	sendMessage(child, message);
	
	// Return the child
	return child;
}

void Manager::restartPage(Child* child) {
	// Move the child to the front of the waiting for socket queue
	children_waiting_for_socket_.removeAll(child);
	children_waiting_for_socket_.push_front(child);
	
	//Make a new process for it
	QProcess* process = new QProcess(this);
	process->setProcessChannelMode(QProcess::ForwardedChannels);
	//connect(process, SIGNAL(started()), SLOT(processStarted()));
	connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished()));
	
	process->start(executable_, QStringList() << kSpawnArgument << server_->serverName());
	processes_ << process;
	
	// Queue a message to create a new page
	NewPage newPage;
	
	SpawnEvent message;
	message.set_type(SpawnEvent_Type_NEW_PAGE);
	message.set_destination(child->id());
	*(message.mutable_new_page()) = newPage;
	
	sendMessage(child, message);
	
	// Load the last page
	child->restoreState();
}

void Manager::destroyPage(Child* child) {
	qDebug() << __PRETTY_FUNCTION__;
	if (children_.contains(child)) {
		// This child was active.  Send it a close message.
		Close close;
		SpawnEvent e;
		e.set_destination(child->id());
		e.set_type(SpawnEvent_Type_CLOSE);
		*(e.mutable_close()) = close;
		
		sendMessage(child, e);
		
		// Remove the child from our list
		QLocalSocket* socket = children_.take(child);
		
		// Now see if it was the last one using its socket.
		// If so, terminate the process.
		bool found = false;
		foreach (QLocalSocket* s, children_.values()) {
			if (s == socket) {
				found = true;
				break;
			}
		}
		
		if (!found) {
			e.clear_destination();
			sendMessage(socket, e);
		}
	} else {
		// Otherwise it must've been waiting for a process.
		// So just remove it
		children_waiting_for_socket_.removeAll(child);
	}
	
	pages_.remove(child->id());
	child->deleteLater();
}

void Manager::newConnection() {
	qDebug() << __PRETTY_FUNCTION__;
	QLocalSocket* socket = server_->nextPendingConnection();
	connect(socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
	connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
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
		qDebug() << "Flusing queued messages for" << child << "to" << socket;
		child->message_queue_.seek(0);
		socket->write(child->message_queue_.readAll());
		child->clearQueue();
	}
}

void Manager::socketDisconnected() {
	qDebug() << __PRETTY_FUNCTION__;
	QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
	
	// Find out if there were any pages using that socket
	QMap<Child*, QLocalSocket*>::iterator i = children_.begin();
	while (i != children_.end()) {
		if (i.value() == socket) {
			Child* child = i.key();
			child->setError();
			
			children_.erase(i);
			children_waiting_for_socket_ << child;
		}
		i++;
	}
	
	// Remove the socket from the list
	sockets_.removeAll(socket);
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

void Manager::sendMessage(QIODevice* dev, const google::protobuf::Message& m) {
	//qDebug() << "Sending message" << m << "to" << dev;
	
	int messageSize = m.ByteSize();
	
	IODeviceOutputStream stream(dev);
	google::protobuf::io::CopyingOutputStreamAdaptor zeroCopyStream(&stream);
	google::protobuf::io::CodedOutputStream codedStream(&zeroCopyStream);
	codedStream.WriteVarint32(messageSize);
	m.SerializeWithCachedSizes(&codedStream);
}

void Manager::sendMessage(Child* child, const google::protobuf::Message& m) {
	if (!child->isReady()) {
		sendMessage(&child->message_queue_, m);
	}
	else {
		sendMessage(children_[child], m);
	}
}

void Manager::processFinished() {
	qDebug() << __PRETTY_FUNCTION__;
	
	QProcess* process = qobject_cast<QProcess*>(sender());
	processes_.removeAll(process);
	//process->deleteLater();
}

void Manager::socketReadyRead() {
	//qDebug() << __PRETTY_FUNCTION__;
	QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
	int bytesRemaining = socket->bytesAvailable();
	
	IODeviceInputStream stream(socket);
	google::protobuf::io::CopyingInputStreamAdaptor zeroCopyStream(&stream);
	google::protobuf::io::CodedInputStream codedStream(&zeroCopyStream);
	
	while (bytesRemaining > 0) {
		quint32 size;
		SpawnReply m;
		
		codedStream.ReadVarint32(&size);
		google::protobuf::io::CodedInputStream::Limit limit = codedStream.PushLimit(size);
		m.ParseFromCodedStream(&codedStream);
		codedStream.PopLimit(limit);
		
		//qDebug() << "Read reply" << m;
		processReply(m);
		
		bytesRemaining -= size + google::protobuf::io::CodedOutputStream::VarintSize32(size);
	}
}

void Manager::processReply(const SpawnReply& reply) {
	quint64 id = reply.source();
	Child* child = pages_[id];

	switch (reply.type()) {
	case SpawnReply_Type_REPAINT_REQUESTED:
	{
		QRect region;
		if (reply.repaint_requested().has_x()) {
			region = QRect(reply.repaint_requested().x(), reply.repaint_requested().y(),
		                       reply.repaint_requested().w(), reply.repaint_requested().h());
		}
		emit child->repaintRequested(region);
		break;
	}
	case SpawnReply_Type_LOAD_PROGRESS:
		emit child->loadProgress(reply.simple_int());
		break;
	case SpawnReply_Type_LOAD_FINISHED:
		emit child->loadFinished(reply.simple_bool());
		break;
	case SpawnReply_Type_LOAD_STARTED:
		emit child->loadStarted();
		break;
	case SpawnReply_Type_STATUS_BAR_MESSAGE:
		emit child->statusBarMessage(QString::fromStdString(reply.simple_string()));
		break;
	case SpawnReply_Type_TITLE_CHANGED:
		emit child->titleChanged(QString::fromStdString(reply.simple_string()));
		break;
	case SpawnReply_Type_URL_CHANGED:
		emit child->urlChanged(QUrl(QString::fromStdString(reply.simple_string())));
		break;
	case SpawnReply_Type_LINK_CLICKED:
		emit child->linkClicked(QUrl(QString::fromStdString(reply.simple_string())));
		break;
	case SpawnReply_Type_SCROLL_REQUESTED:
		emit child->scrollRequested(reply.scroll_requested().dx(), reply.scroll_requested().dy(),
		                            QRect(reply.scroll_requested().x(), reply.scroll_requested().y(),
		                                  reply.scroll_requested().w(), reply.scroll_requested().h()));
		break;
	
	default:
		break;
	}
}


}
