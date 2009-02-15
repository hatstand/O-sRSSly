#include "config.h"
#include "keychain.h"

#ifdef Q_OS_DARWIN
#include <Security/Security.h>
#else
#if !defined(NO_GNOME_KEYRING) && defined (Q_OS_UNIX)
const GnomeKeyringPasswordSchema Keychain::sOurSchema = {
	GNOME_KEYRING_ITEM_GENERIC_SECRET,
	{
		{ "username", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "service", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ NULL }
	},
};
#endif
#if !defined(NO_KWALLET) && defined(Q_OS_UNIX)
#include "kwallet.h"
#include <QDBusConnection>
#include <QDBusPendingReply>
#endif
#endif

QString Keychain::password_;

#include <QDebug>

const QString Keychain::kServiceName = "Purplehatstands-" TITLE;
const QString Keychain::kKWalletServiceName = "org.kde.kwalletd";
const QString Keychain::kKWalletPath = "/modules/kwalletd";

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
#elif !defined(NO_KWALLET) && defined(Q_OS_UNIX)
	// See if KWallet is on the session bus, otherwise we can try gnome.
	// TODO: Asynchronous?
	org::kde::KWallet kwallet(kKWalletServiceName, kKWalletPath, QDBusConnection::sessionBus());
	if (kwallet.isValid()) {
		QDBusPendingReply<bool> check = kwallet.isEnabled();
		check.waitForFinished();
		Q_ASSERT(check.isValid());
		QDBusPendingReply<QString> wallet_name = kwallet.networkWallet();
		wallet_name.waitForFinished();
		if (wallet_name.isValid()) {
			qDebug() << "Wallet:" << wallet_name.value();
			QDBusPendingReply<int> reply = kwallet.open(wallet_name.value(), 0, kServiceName);
			reply.waitForFinished();
			qDebug() << reply.value();
	
			QDBusPendingReply<QString> password = kwallet.readPassword(reply.value(), "Passwords", account, kServiceName);
			password.waitForFinished();
			Q_ASSERT(password.isValid());
	
			if (!password.value().isEmpty())
				return password.value();
		}
	}
	// Try gnome instead if we compiled support for it.
#elif !defined(NO_GNOME_KEYRING) && defined (Q_OS_UNIX)
	if (gnome_keyring_is_available()) {
		GnomeKeyringResult result = gnome_keyring_find_password_sync(
			&sOurSchema,
			&password,
			"username", account.toStdString().c_str(),
			"service", kServiceName.toStdString().c_str(),
			NULL);
		
		if (result == GNOME_KEYRING_RESULT_OK) {
			QString pass(password);
			gnome_keyring_free_password(password);
			return pass;
		}
	}
#else
	Q_UNUSED(password);
#endif
	// If we got here then kde/gnome/keychain all failed.
	return password_;
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

		if (ret != 0) {
			qWarning() << "Error setting password in keychain";
		} else {
			// Successful :-)
			return;
		}
	} else {
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

		if (ret != 0) {
			qWarning() << "Error setting password in keychain";
		} else {
			// Successful :-)
			return;
		}
	}
#elif !defined(NO_KWALLET) && defined(Q_OS_UNIX)
	// KWallet tried first as if the user is not running KDE, then it won't show up on DBus.
	// Gnome keyring probably works in KDE anyway.
	org::kde::KWallet kwallet(kKWalletServiceName, kKWalletPath, QDBusConnection::sessionBus());
	if (kwallet.isValid()) {
		QDBusPendingReply<QString> wallet_name = kwallet.networkWallet();
		wallet_name.waitForFinished();
		Q_ASSERT(wallet_name.isValid());
		QDBusPendingReply<int> handle = kwallet.open(wallet_name.value(), 0, kServiceName);
		handle.waitForFinished();
		Q_ASSERT(handle.isValid());
		qDebug() << handle.value();

		QDBusPendingReply<int> ret = kwallet.writePassword(handle.value(), "Passwords", account, password, kServiceName);
		ret.waitForFinished();
		Q_ASSERT(ret.isValid());
		// Successful :-)
		return;
	}
#elif !defined(NO_GNOME_KEYRING) && defined(Q_OS_UNIX)
	if (gnome_keyring_is_available()) {
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
			// Uses the default implementation on failure.
		} else {
			// Successful :-)
			return;
		}
	}
#endif
	// Default implementation if all others fail at runtime.
	password_ = password;
}
