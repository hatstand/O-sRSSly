#ifndef SPAWNMANAGER_H
#define SPAWNMANAGER_H

#include <QProcess>
#include <QObject>
#include <QQueue>
#include <QMap>

#include "mouseevent.pb.h"

namespace Spawn {

class Child;

class Manager : public QObject {
	Q_OBJECT
public:
	Manager(QObject* parent = 0, const QString& executable = QString::null);
	~Manager();
	
	Child* createPage();
	void destroyPage(Child* child);
	
	void sendEvent(MouseEvent* e);

private slots:
	void processStarted();
	void processError(QProcess::ProcessError error);
	
	void processReadyReadStderr();
	
private:
	QMap<Q_PID, QProcess*> processes_;
	QMap<Child*, Q_PID> children_;
	QQueue<Child*> children_waiting_for_process_;
	
	quint64 next_child_id_;
	
	QString executable_;
};

}

#endif
