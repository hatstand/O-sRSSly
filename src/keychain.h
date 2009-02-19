#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include <boost/utility.hpp>

#include <QString>

class Keychain : boost::noncopyable {
public:
	virtual ~Keychain() {}
	virtual bool isAvailable() = 0;

	virtual const QString getPassword(const QString& account) = 0;
	virtual bool setPassword(const QString& account, const QString& password) = 0;

	virtual const QString& implementationName() const = 0;

	static Keychain* getDefault();

private:
	static const QString kServiceName;

	// Fun for all the family.
	struct KeychainDefinition {
		virtual ~KeychainDefinition() {}
		const char* getName() const { return name_; }
		virtual Keychain* getInstance() const = 0;
	protected:
		const char* name_;
	};
	template<typename T>
	struct KeychainImpl : public KeychainDefinition {
		KeychainImpl(const char* name) { name_ = name; }
		virtual Keychain* getInstance() const {
			return new T();
		}
	};

	static const KeychainDefinition* kCompiledImplementations[];
};

#endif
