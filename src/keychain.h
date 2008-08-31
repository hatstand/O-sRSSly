#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include <QString>

class Keychain {
public:
	static QString getPassword(QString account);
	static void setPassword(QString account, QString password);

private:
	static const QString kServiceName;
};

#endif
