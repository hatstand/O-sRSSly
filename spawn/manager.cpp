#include "manager.h"
#include "child.h"

#include <QtDebug>
#include <QApplication>
#include <iostream>

namespace Spawn {

Manager::Manager(QObject* parent, const QString& executable)
	: QObject(parent),
	  next_child_id_(0),
	  executable_(executable)
{
	if (executable_.isNull()) {
		executable_ = QApplication::arguments()[0];
	}
}

Manager::~Manager() {
}

Child* Manager::createPage() {
	qDebug() << __PRETTY_FUNCTION__;
	// For now each page has its own process, but later we might want
	// to have some pages sharing - in that case just send a NewPage
	// message to an existing process
	
	Child* child = new Child(this, next_child_id_++);
	children_waiting_for_process_.enqueue(child);
	
	QProcess* process = new QProcess(this);
	connect(process, SIGNAL(started()), SLOT(processStarted()));
	connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(readyReadStandardError()), SLOT(processReadyReadStderr()));
	
	process->start(executable_, QStringList() << "--spawn");
	child->starting_process_ = process;
	return child;
}

void Manager::destroyPage(Child* child) {
	qDebug() << __PRETTY_FUNCTION__;
	if (child->starting_process_) {
		// This child was starting a new process and hadn't finished yet
		QProcess* process = child->starting_process_;
		process->disconnect();
		process->kill();
		process->waitForFinished(-1); // TODO: Don't block
		delete process;
		children_waiting_for_process_.removeAll(child);
		delete child;
		return;
	}
	
	// Now we have a one-to-one mapping between children and processes,
	// so just delete the process;
	Q_PID pid = children_[child];
	delete processes_[pid]; // TODO: Close it properly
	processes_.remove(pid);
	children_.remove(child);
	delete child;
}

void Manager::processStarted() {
	qDebug() << __PRETTY_FUNCTION__;
	QProcess* process = qobject_cast<QProcess*>(sender());
	
	if (children_waiting_for_process_.isEmpty()) {
		// This shouldn't ever happen - terminate the process
		process->close();
		return;
	}
	
	Child* child = children_waiting_for_process_.dequeue();
	Q_PID pid = process->pid();
	
	processes_[pid] = process;
	children_[child] = pid;
	child->setReady();
}

void Manager::processError(QProcess::ProcessError error) {
	qDebug() << __PRETTY_FUNCTION__;
	switch (error) {
	case QProcess::FailedToStart:
		if (children_waiting_for_process_.isEmpty()) {
			// Oh well
			return;
		}
		
		delete children_waiting_for_process_.dequeue();
		break;
	default:
		qDebug() << "Process error" << error;
		break;
	}
}

void Manager::processReadyReadStderr() {
	QProcess* process = qobject_cast<QProcess*>(sender());
	QByteArray data(process->readAllStandardError());
	
	std::cout << data.constData();
}



}
