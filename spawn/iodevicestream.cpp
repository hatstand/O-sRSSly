#include "iodevicestream.h"

#include <QIODevice>

namespace Spawn {

IODeviceInputStream::IODeviceInputStream(QIODevice* dev)
	: dev_(dev)
{
}

int IODeviceInputStream::Read(void* buffer, int size) {
	return dev_->read(reinterpret_cast<char*>(buffer), size);
}

IODeviceOutputStream::IODeviceOutputStream(QIODevice* dev)
	: dev_(dev)
{
}

bool IODeviceOutputStream::Write(const void* buffer, int size) {
	return dev_->write(reinterpret_cast<const char*>(buffer), size);
}

}
