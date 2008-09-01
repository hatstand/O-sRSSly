#include "keychain.h"

#ifdef Q_OS_DARWIN
#include <Security/Security.h>
#elif defined (Q_OS_UNIX)
extern "C"{
#include <gnome-keyring.h>
}

static GnomeKeyringPasswordSchema ourSchema = {
	GNOME_KEYRING_ITEM_GENERIC_SECRET, 
	{
		{ "username", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "service", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ NULL }
	},
};
#endif

#include <QDebug>

const QString Keychain::kServiceName = "Purplehatstands-Feeder";

QString Keychain::getPassword(QString account) {
	char* password;
#ifdef Q_OS_DARWIN
	UInt32 password_length;
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
	else {
		QString pass = QString::fromAscii(password, password_length);
		SecKeychainItemFreeContent(NULL, password);
		return pass;
	}
#elif defined (Q_OS_UNIX)
	GnomeKeyringResult result = gnome_keyring_find_password_sync(
		&ourSchema,
		&password,
		"username", account.toStdString().c_str(),
		"service", kServiceName.toStdString().c_str(),
		NULL);
	
	if (result != GNOME_KEYRING_RESULT_OK) {
		return QString::null;
	}
	else{
		QString pass = QString(password);
		gnome_keyring_free_password(password);
		return pass;
	}
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
#elif defined (Q_OS_UNIX)
    QString displayName=("Feeder Google Reader account for ");
    displayName.append(account);
	GnomeKeyringResult result = gnome_keyring_store_password_sync(
		&ourSchema, NULL,
		displayName.toStdString().c_str(),
		password.toStdString().c_str(),
		"username", account.toStdString().c_str(),
		"service", kServiceName.toStdString().c_str(),
		NULL);
	if (result != GNOME_KEYRING_RESULT_OK){
		qWarning() << "Error setting password in keychain";
	}
#else
	return;
#endif
}
