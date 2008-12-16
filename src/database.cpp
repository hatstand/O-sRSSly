#include "database.h"

#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QtDebug>

Database::Database(QObject* parent)
	: QThread(parent),
	  stopping_(false)
{
	// Initialisation done in run() to put it in separate thread.
}

void Database::initTables()
{
	QFile file(":inittables.sql");
	file.open(QIODevice::ReadOnly);
	
	QString fileContents(file.readAll());
	QStringList queries(fileContents.split("\n\n", QString::SkipEmptyParts));
	
	foreach (const QString& query, queries)
	{
		QSqlQuery q;
		q.prepare(query);
		
		if (!q.exec())
			qFatal("Error executing SQL query: %s", query.toAscii().data());
	}
}

void Database::handleError(const QSqlError& error) {
	qDebug() << "Database error:" << error;
}

void Database::stop() {
	{
		QMutexLocker locker(&mutex_);
		stopping_ = true;
	}
	wait_condition_.wakeAll();
}

void Database::run() {
	qDebug() << __PRETTY_FUNCTION__;
	// TODO: Put this in a better place
	QString path(QDir::homePath() + "/.feeder.sqlite");
	

	// This is all done here to put it in the right thread.
	QSqlDatabase db(QSqlDatabase::addDatabase("QSQLITE"));
	db.setDatabaseName(path);
	if (!db.open())
		qFatal("Could not open database");
	// TODO: Die more gracefully
	
	initTables();

	// Initial lock & check to make sure we don't miss anything on startup.
	mutex_.lock();
	forever {
		db.transaction();
		while (!queries_.empty()) {
			// Grab one from the queue.
			Request req = queries_.dequeue();
			// Unlock so we don't block on the db.exec().
			mutex_.unlock();
			QSqlQuery query;
			query.prepare(req.query);
			foreach (const QVariant& v, req.bind_values) {
				query.addBindValue(v);
			}
			if (!query.exec()) {
				handleError(db.lastError());
				qDebug() << query.executedQuery();
			} else {
				if (req.cb)
					req.cb(query);
			}
			// Grab lock to check queue again.
			// If it's empty, then we lose the lock at the end of the loop.
			mutex_.lock();
		}
		db.commit();

		// Quit thread.
		if (stopping_) {
			qDebug() << __PRETTY_FUNCTION__ << "quitting...";
			mutex_.unlock();
			return;
		}
		
		// Unlock mutex until something happens.
		wait_condition_.wait(&mutex_);
		// Mutex is now locked.
	}
}

void Database::pushQuery(const QString& query, DatabaseCallback cb) {
	Request req = { query, QList<QVariant>(), cb };
	pushQuery(req);
}

void Database::pushQuery(const QString& query, const QList<QVariant>& bind_values, DatabaseCallback cb) {
	Request req = { query, bind_values, cb };
	pushQuery(req);
}

void Database::pushQuery(const Request& request) {
	{
		// Lock and add to queue.
		QMutexLocker locker(&mutex_);
		queries_.enqueue(request);
	}
	// Notify threads that there's some work to do.
	wait_condition_.wakeAll();
}

