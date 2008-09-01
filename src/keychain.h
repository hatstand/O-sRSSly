#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include <QString>

#ifdef Q_OS_UNIX
extern "C" {
#include <gnome-keyring.h>
}
#endif

class Keychain {
public:
	static QString getPassword(QString account);
	static void setPassword(QString account, QString password);

private:
	static const QString kServiceName;
	
#ifdef Q_OS_UNIX
	static const GnomeKeyringPasswordSchema sOurSchema;
#endif
};

#endif
