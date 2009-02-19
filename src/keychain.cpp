#include "config.h"
#include "keychain.h"

#include "default_keychain.h"

const QString Keychain::kServiceName = "Purplehatstands-" TITLE;

const Keychain::KeychainDefinition* Keychain::kCompiledImplementations[] = {
#ifdef Q_OS_DARWIN
	new KeychainImpl<MacKeychain>("OS X Keychain"),
#elif defined(Q_OS_LINUX)
	#ifndef NO_KWALLET
	new KeychainImpl<KWalletKeychain>("KWallet"),
	#endif
	#ifndef NO_GNOME_KEYRING
	new KeychainImpl<GnomeKeychain>("Gnome Keyring"),
	#endif
#endif
	new KeychainImpl<DefaultKeychain>("Default"),
	NULL
};

Keychain* Keychain::getDefault() {
	const KeychainDefinition** ptr = kCompiledImplementations;
	while (*ptr != NULL) {
		Keychain* keychain = (*ptr)->getInstance();
		if (keychain->isAvailable()) {
			return keychain;
		} else {
			delete keychain;
		}
	}

	return NULL;
}
