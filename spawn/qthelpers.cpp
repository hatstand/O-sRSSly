#include "qthelpers.h"

QDebug operator <<(QDebug d, const google::protobuf::Message& m) {
	std::string debug(m.ShortDebugString());
	d << debug.c_str();
	return d.space();
}

QIODevice& operator <<(QIODevice& d, const std::string& s) {
	d.write(s.c_str(), s.size());
	return d;
}
