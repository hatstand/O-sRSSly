#include "jsonutils.h"

using JsonUtils::JsonObject;

JsonUtils::JsonObject::JsonObject() {

}

JsonUtils::JsonObject::~JsonObject() {
	for (QMultiMap<QString, JsonObject*>::const_iterator it = objects_.begin();
		it != objects_.end(); ++it) {
		delete it.value();
	}
}

void JsonUtils::JsonObject::addItem(const QString& key, JsonObject* value) {
	objects_.insert(key, value);
}

void JsonUtils::JsonObject::addItem(const QString& key, int value) {
	integers_.insert(key, value);
}

void JsonUtils::JsonObject::addItem(const QString& key, double value) {
	reals_.insert(key, value);
}

void JsonUtils::JsonObject::addItem(const QString& key, bool value) {
	bools_.insert(key, value);
}

void JsonUtils::JsonObject::addItem(const QString& key, const QString& value) {
	strings_.insert(key, value);
}


QList<JsonObject*> JsonUtils::JsonObject::getObject(const QString& key) const {
	return objects_.values(key);
}

QList<int> JsonUtils::JsonObject::getInt(const QString& key) const {
	return integers_.values(key);
}

QList<double> JsonUtils::JsonObject::getDouble(const QString& key) const {
	return reals_.values(key);
}

QList<bool> JsonUtils::JsonObject::getBool(const QString& key) const {
	return bools_.values(key);
}

QList<QString> JsonUtils::JsonObject::getString(const QString& key) const {
	return strings_.values(key);
}

JsonObject* JsonUtils::parseJson(const QByteArray& data) {
	qDebug() << __PRETTY_FUNCTION__;

	std::string json(data);
	json::grammar<char>::variant v = json::parse(json.begin(), json.end());

	return traverseJson(v);
}

JsonObject* JsonUtils::traverseJson(const json::grammar<char>::variant& var) {

	if (var->type() == typeid(json::grammar<char>::object)) {

		// Create a new JSON object.
		const json::grammar<char>::object& o = boost::any_cast<json::grammar<char>::object>(*var);
		JsonObject* object = new JsonObject();

		// Iterate through and add its properties.
		for (json::grammar<char>::object::const_iterator it = o.begin(); it != o.end(); ++it) {
			QString key = QString::fromUtf8(it->first.c_str());
			const json::grammar<char>::variant& var_value = it->second;

			if (var_value->type() == typeid(int)) {
				int value = boost::any_cast<int>(*var_value);
				object->addItem(key, value);
			} else if (var_value->type() == typeid(std::string)) {
				std::string value = boost::any_cast<std::string>(*var_value);
				object->addItem(key, QString::fromUtf8(value.c_str()));
			} else if (var_value->type() == typeid(json::grammar<char>::array)) {
				const json::grammar<char>::array& a =
					boost::any_cast<json::grammar<char>::array>(*var_value);
				for (json::grammar<char>::array::const_iterator it = a.begin(); it != a.end(); ++it) {
					JsonObject* value = traverseJson(*it);
					if (value)
						object->addItem(key, value);
				}
			} else {
				JsonObject* value = traverseJson(it->second);
				if (value)
					object->addItem(key, value);
			}
		}

		return object;
	}

	// Shouldn't happen.
	return NULL;
}

QDebug operator <<(QDebug dbg, const JsonObject& o) {
	dbg.nospace() << "JsonObject(\n";

	for (QMultiMap<QString, JsonObject*>::const_iterator it = o.getObjects().begin();
		it != o.getObjects().end(); ++it) {
		dbg.nospace() << it.key() << ":" << *it.value() << "\n";
	}
	for (QMultiMap<QString, int>::const_iterator it = o.getInts().begin();
		it != o.getInts().end(); ++it) {
		dbg.nospace() << it.key() << ":" << it.value() << "\n";
	}
	for (QMultiMap<QString, double>::const_iterator it = o.getDoubles().begin();
		it != o.getDoubles().end(); ++it) {
		dbg.nospace() << it.key() << ":" << it.value() << "\n";
	}
	for (QMultiMap<QString, bool>::const_iterator it = o.getBools().begin();
		it != o.getBools().end(); ++it) {
		dbg.nospace() << it.key() << ":" << it.value() << "\n";
	}
	for (QMultiMap<QString, QString>::const_iterator it = o.getStrings().begin();
		it != o.getStrings().end(); ++it) {
		dbg.nospace() << it.key() << ":" << it.value() << "\n";
	}

	dbg.nospace() << ")\n";

	return dbg.space();
}
