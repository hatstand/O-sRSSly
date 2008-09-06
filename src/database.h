#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class Database
{
public:
	Database();
	
	static void handleError(const QSqlError& error);
	
private:
	void initTables();
};


#endif

