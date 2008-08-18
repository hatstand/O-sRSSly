#include "database.h"

#include <QDir>
#include <QFile>
#include <QSqlQuery>

Database::Database()
{
	// TODO: Put this in a better place
	QString path(QDir::homePath() + "/.feeder.sqlite");
	
	QSqlDatabase db(QSqlDatabase::addDatabase("QSQLITE"));
	db.setDatabaseName(path);
	if (!db.open())
		qFatal("Could not open database");
	// TODO: Die more gracefully
	
	initTables();
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

