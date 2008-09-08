#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include <QString>

#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
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
	
#if !defined(NO_KEYRING) && defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	static const GnomeKeyringPasswordSchema sOurSchema;
#endif

#ifdef NO_KEYRING
	static QString password_;
#endif
};

#endif
