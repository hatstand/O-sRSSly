#include "mapped_memory.h"

#include <cstdio>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

#include <QTemporaryFile>
#include <QDir>
#include <QtDebug>

MappedMemory::MappedMemory()
	: tmp_file_(QDir::tempPath())
{
	tmp_file_.open();
	key_ = tmp_file_.fileName();

	init(tmp_file_.handle());
}

MappedMemory::MappedMemory(const QString& key)
	: key_(key) {
	QFile file(key);
	file.open(QIODevice::ReadWrite);

	init(file.handle());
}

void MappedMemory::init(int fd) {
	length_ = 8*1024*1024;
	if (ftruncate(fd, length_) != 0) {
		qFatal(strerror(errno));
	}

	void* data = mmap(NULL, length_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if ((qint64)data == -1) {
		qFatal(strerror(errno));
	}

	data_ = static_cast<char*>(data);
	qDebug() << int(data_);
}

MappedMemory::~MappedMemory() {
	munmap(data_, length_);
}

bool MappedMemory::lock() {
	// TODO
}

bool MappedMemory::unlock() {
	// TODO
}

char* MappedMemory::data() {
	return data_;
}

const char* MappedMemory::data() const {
	return data_;
}

QString MappedMemory::key() const {
	return key_;
}

void MappedMemory::resize(quint64 size) {
	// TODO
}

