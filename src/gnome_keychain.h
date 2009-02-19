#ifndef GNOME_KEYCHAIN_H
#define GNOME_KEYCHAIN_H

#include "keychain.h"

struct GnomeKeyringPassowrdSchema;

class GnomeKeychain : public Keychain {
public:
	virtual ~GnomeKeychain() {}
	virtual bool isAvailable();
	virtual const QString getPassword(const QString& account);
	virtual bool setPassword(const QString& account, const QString& password);

	virtual const QString& implementationName() const { return kImplementationName; }
private:
	static const QString kImplementationName;
	static const GnomeKeyringPasswordScheme kOurSchema;
};

#endif
