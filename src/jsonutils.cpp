#include "jsonutils.h"

using JsonUtils::JsonObject;

JsonUtils::JsonObject::JsonObject() {

}

JsonUtils::JsonObject::~JsonObject() {
	for (QMultiMap<QString, JsonObject*>::const_iterator it = objects_.begin(); it != objects_.end(); ++it) {
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


JsonObject* JsonUtils::JsonObject::getObject(const QString& key) const {
	QMultiMap<QString, JsonObject*>::const_iterator it = objects_.find(key);
	if (it != objects_.end())
		return it.value();
	else
		return NULL;
}

int JsonUtils::JsonObject::getInt(const QString& key) const {
	QMultiMap<QString, int>::const_iterator it = integers_.find(key);
	if (it != integers_.end())
		return it.value();
	else
		return NULL;
}

double JsonUtils::JsonObject::getDouble(const QString& key) const {
	QMultiMap<QString, double>::const_iterator it = reals_.find(key);
	if (it != reals_.end())
		return it.value();
	else
		return NULL;
}

bool JsonUtils::JsonObject::getBool(const QString& key) const {
	QMultiMap<QString, bool>::const_iterator it = bools_.find(key);
	if (it != bools_.end())
		return it.value();
	else
		return NULL;
}

QString JsonUtils::JsonObject::getString(const QString& key) const {
	QMultiMap<QString, QString>::const_iterator it = strings_.find(key);
	if (it != strings_.end())
		return it.value();
	else
		return NULL;
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
		qDebug() << "JSON: New object:";
		JsonObject* object = new JsonObject();

		// Iterate through and add its properties.
		for (json::grammar<char>::object::const_iterator it = o.begin(); it != o.end(); ++it) {
			qDebug() << "JSON: <object element>" << it->first.c_str();
			QString key = QString::fromUtf8(it->first.c_str());
			const json::grammar<char>::variant& var_value = it->second;

			if (var_value->type() == typeid(int)) {
				int value = boost::any_cast<int>(*var_value);
				qDebug() << "JSON: int" << value;
				object->addItem(key, value);
			} else if (var_value->type() == typeid(std::string)) {
				std::string value = boost::any_cast<std::string>(*var_value);
				qDebug() << "JSON: string" << value.c_str();
				object->addItem(key, QString::fromUtf8(value.c_str()));
			} else if (var_value->type() == typeid(json::grammar<char>::array)) {
				qDebug() << "JSON: New array:";
				const json::grammar<char>::array& a =
					boost::any_cast<json::grammar<char>::array>(*var_value);
				for (json::grammar<char>::array::const_iterator it = a.begin(); it != a.end(); ++it) {
					qDebug() << "JSON: <array element>";
					JsonObject* value = traverseJson(*it);
					object->addItem(key, value);
					qDebug() << "JSON: </array element>";
				}
			} else {
				JsonObject* value = traverseJson(it->second);
				object->addItem(key, value);
			}
			qDebug() << "JSON: </object element>";
		}
		qDebug() << "JSON: End object";

		return object;
	}

	// Shouldn't happen.
	return NULL;
}

QDebug operator <<(QDebug dbg, const JsonObject& o) {
	dbg.nospace() << "JsonObject(\n";

	for (QMultiMap<QString, JsonObject*>::const_iterator it = o.getObjects().begin();
		it != o.getObjects().end(); ++it) {
		dbg.nospace() << it.key() << ":" << it.value() << "\n";
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
