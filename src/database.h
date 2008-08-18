#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>

class Database
{
public:
	Database();
	
private:
	void initTables();
	
	QSqlDatabase m_db;
};


#endif

