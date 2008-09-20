#ifndef QTHELPERS_H
#define QTHELPERS_H

#include <QtDebug>
#include <google/protobuf/message.h>

QDebug operator <<(QDebug d, const google::protobuf::Message& m);
QIODevice& operator <<(QIODevice& d, const std::string& s);

#endif
