#ifndef IODEVICESTREAM_H
#define IODEVICESTREAM_H

#include <google/protobuf/io/zero_copy_stream_impl.h>

class QIODevice;

namespace Spawn {

class IODeviceInputStream : public google::protobuf::io::CopyingInputStream {
public:
	IODeviceInputStream(QIODevice* dev);
	
	int Read(void* buffer, int size);

private:
	QIODevice* dev_;
};

class IODeviceOutputStream : public google::protobuf::io::CopyingOutputStream {
public:
	IODeviceOutputStream(QIODevice* dev);
	
	bool Write(const void* buffer, int size);

private:
	QIODevice* dev_;
};

}

#endif
