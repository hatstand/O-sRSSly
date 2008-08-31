#include "keychain.h"

#ifdef Q_OS_DARWIN
#include <Security/Security.h>
#endif

#include <QDebug>

const QString Keychain::kServiceName = "Purplehatstands-Feeder";

QString Keychain::getPassword(QString account) {
#ifdef Q_OS_DARWIN
	UInt32 password_length;
	char* password;
	OSStatus ret = SecKeychainFindGenericPassword(
		NULL,
		kServiceName.length(),
		kServiceName.toStdString().c_str(),
		account.length(),
		account.toStdString().c_str(),
		&password_length,
		(void**)&password,
		NULL);
	
	if (ret != 0)
		return QString::null;
	else
		return QString::fromAscii(password, password_length);
#else
	return QString::null;
#endif
}

void Keychain::setPassword(QString account, QString password) {
#ifdef Q_OS_DARWIN
	OSStatus ret = SecKeychainAddGenericPassword(
		NULL,
		kServiceName.length(),
		kServiceName.toStdString().c_str(),
		account.length(),
		account.toStdString().c_str(),
		password.length(),
		password.toStdString().c_str(),
		NULL);

	if (ret != 0)
		qWarning() << "Error setting password in keychain";
#else
	return;
#endif
}
