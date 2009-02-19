#ifndef KWALLET_KEYCHAIN_H
#define KWALLET_KEYCHAIN_H

#include "keychain.h"

#include "kwallet.h"

class KWalletKeychain : public Keychain {
public:
	KWalletKeychain();
	virtual ~KWalletKeychain();

	virtual const QString getPassword(const QString& account);
	virtual bool getPassword(const QString& account, const QString& password);

	virtual bool isAvailable();

	virtual const QString& implementationName() const { return kImplementationName; }

private:
	org::kde::KWallet kwallet_;
	int handle_;

	static const QString kImplementationName;
	static const QString kKWalletServiceName;
	static const QString kKWalletPath;
	static const QString kKWalletFolder;
};

#endif
