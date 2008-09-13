#ifndef JSONUTILS_H
#define JSONUTILS_H

#include "../tinyjson/tinyjson.hpp"

#include <QDebug>
#include <QMultiMap>
#include <QString>

namespace JsonUtils {

class JsonObject {
public:
	JsonObject();
	~JsonObject();
	void addItem(const QString& key, JsonObject* value);
	void addItem(const QString& key, int value);
	void addItem(const QString& key, double value);
	void addItem(const QString& key, bool value);
	void addItem(const QString& key, const QString& value);

	JsonObject* getObject(const QString& key) const;
	int getInt(const QString& key) const;
	double getDouble(const QString& key) const;
	bool getBool(const QString& key) const;
	QString getString(const QString& key) const;

	const QMultiMap<QString, JsonObject*>& getObjects() const { return objects_; }
	const QMultiMap<QString, int>& getInts() const { return integers_; }
	const QMultiMap<QString, double>& getDoubles() const { return reals_; }
	const QMultiMap<QString, bool>& getBools() const { return bools_; }
	const QMultiMap<QString, QString>& getStrings() const { return strings_; }
private:
	QMultiMap<QString, JsonObject*> objects_;
	QMultiMap<QString, int> integers_;
	QMultiMap<QString, double> reals_;
	QMultiMap<QString, bool> bools_;
	QMultiMap<QString, QString> strings_;
};

JsonObject* parseJson(const QByteArray& data);
JsonObject* traverseJson(const json::grammar<char>::variant& v); 

};

QDebug operator <<(QDebug dbg, const JsonUtils::JsonObject& o);

#endif
