#ifndef DATABASE_H
#define DATABASE_H

#include <QMutex>
#include <QQueue>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QVariant>
#include <QWaitCondition>

#include <boost/function.hpp>

using boost::function;

typedef function<void (const QSqlQuery&)> DatabaseCallback;

class Database : public QThread
{
	Q_OBJECT
public:
	Database(QObject* parent = 0);

	static void handleError(const QSqlError& error);

	void run();
	void stop();

	void getFolderItems();
	void getFeedItems();
	void getCategories(const QString& id);
	void getEntries(const QString& id);

	void pushQuery(const QString& query, DatabaseCallback cb = DatabaseCallback());
	void pushQuery(const QString& query, const QList<QVariant>& bind_values, DatabaseCallback cb = DatabaseCallback());

private:
	void initTables();

	struct Request {
		QString query;
		QList<QVariant> bind_values;
		DatabaseCallback cb;
	};

	void pushQuery(const Request& request);

	QQueue<Request> queries_;
	QMutex mutex_;
	QWaitCondition wait_condition_;

	bool stopping_;
};


#endif

