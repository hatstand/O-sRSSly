#ifndef SPAWNMANAGER_H
#define SPAWNMANAGER_H

#include <QProcess>
#include <QObject>
#include <QQueue>
#include <QList>
#include <QMap>

#include <google/protobuf/message.h>

class QLocalSocket;
class QLocalServer;

namespace Spawn {

class Child;
class SpawnReply;

class Manager : public QObject {
	Q_OBJECT
	friend class Child;
public:
	Manager(QObject* parent = 0, const QString& executable = QString::null);
	~Manager();
	
	Child* createPage();
	void destroyPage(Child* child);
	
	static QString serverName();
	static const char* const kSpawnArgument;

private slots:
	void newConnection();
	void processError(QProcess::ProcessError error);
	void processFinished();
	void socketDisconnected();
	void socketReadyRead();
	
private:
	// To be used by Child
	void sendMessage(Child* child, const google::protobuf::Message& m);
	
	// For internal use
	void sendMessage(QIODevice* dev, const google::protobuf::Message& m);
	void processReply(const SpawnReply& reply);
	
	QLocalServer* server_;
	QList<QLocalSocket*> sockets_;
	QList<QProcess*> processes_;
	QMap<Child*, QLocalSocket*> children_;
	QMap<quint64, Child*> pages_;
	QQueue<Child*> children_waiting_for_socket_;
	
	quint64 next_child_id_;
	
	QString executable_;
};

}

#endif
