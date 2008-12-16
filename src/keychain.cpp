#include "config.h"
#include "keychain.h"

#ifdef Q_OS_DARWIN
#include <Security/Security.h>
#elif !defined(NO_KEYRING) && defined (Q_OS_UNIX)
const GnomeKeyringPasswordSchema Keychain::sOurSchema = {
	GNOME_KEYRING_ITEM_GENERIC_SECRET,
	{
		{ "username", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "service", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ NULL }
	},
};
#endif

QString Keychain::password_;

#include <QDebug>

const QString Keychain::kServiceName = "Purplehatstands-" TITLE;

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
#elif !defined(NO_KEYRING) && defined (Q_OS_UNIX)
	if (!gnome_keyring_is_available()) {
		return password_;
	} else {
		GnomeKeyringResult result = gnome_keyring_find_password_sync(
			&sOurSchema,
			&password,
			"username", account.toStdString().c_str(),
			"service", kServiceName.toStdString().c_str(),
			NULL);
		
		if (result != GNOME_KEYRING_RESULT_OK) {
			return QString::null;
		}
		else {
			QString pass(password);
			gnome_keyring_free_password(password);
			return pass;
		}
	}
#else
	Q_UNUSED(password);
	return password_;
#endif
}

void Keychain::setPassword(QString account, QString password) {
#ifdef Q_OS_DARWIN
	// If password exists, just update.
	SecKeychainItemRef item;
	OSStatus ret = SecKeychainFindGenericPassword(
		NULL,
		kServiceName.length(),
		kServiceName.toStdString().c_str(),
		account.length(),
		account.toStdString().c_str(),
		NULL, // Don't care about the actual password.
		NULL,
		&item);
	if (ret == 0) {
		OSStatus ret = SecKeychainItemModifyAttributesAndData(
			item,
			NULL,
			password.length(),
			password.toStdString().c_str());

		if (ret != 0)
			qWarning() << "Error setting password in keychain";

		return;
	}

	// Password not already there. Create new one.
	ret = SecKeychainAddGenericPassword(
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
#elif !defined(NO_KEYRING) && defined (Q_OS_UNIX)
	if (!gnome_keyring_is_available()) {
		password_ = password;
	} else {
		QString displayName=(TITLE " Google Reader account for ");
		displayName.append(account);
		GnomeKeyringResult result = gnome_keyring_store_password_sync(
			&sOurSchema, NULL,
			displayName.toStdString().c_str(),
			password.toStdString().c_str(),
			"username", account.toStdString().c_str(),
			"service", kServiceName.toStdString().c_str(),
			NULL);
		if (result != GNOME_KEYRING_RESULT_OK) {
			qWarning() << "Error setting password in keychain";
		}
	}
#else
	password_ = password;
#endif
}
