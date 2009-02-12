#include "mapped_memory.h"

#include <cstdio>
#include <sys/mman.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>

#include <QTemporaryFile>
#include <QDir>
#include <QtDebug>

MappedMemory::MappedMemory()
	: file_(new QTemporaryFile(QDir::tempPath() + QDir::separator() + "XXXXXX"))
{
	length_ = 8*1024*1024;
	file_->open(QIODevice::ReadWrite);
	if (!file_->resize(length_)) {
		qFatal("Mapped file resize failed");
	}
	init();
}

MappedMemory::MappedMemory(const QString& key)
	: file_(new QFile(key))
{
	file_->open(QIODevice::ReadWrite);
	length_ = file_->size();
	init();
}

void MappedMemory::init() {
	Q_ASSERT(length_ == file_->size());
	void* data = mmap(NULL, length_, PROT_READ | PROT_WRITE, MAP_SHARED, file_->handle(), 0);
	if ((qint64)data == -1) {
		qFatal(strerror(errno));
	}

	data_ = static_cast<char*>(data);
	qDebug() << (quintptr)data_;
}

MappedMemory::~MappedMemory() {
	munmap(data_, length_);
}

bool MappedMemory::lock() {
	if (flock(file_->handle(), LOCK_EX) != 0) {
		qFatal(strerror(errno));
	}
	return true;
}

bool MappedMemory::unlock() {
	if (flock(file_->handle(), LOCK_UN) != 0) {
		qFatal(strerror(errno));
	}
	return true;
}

char* MappedMemory::data() {
	return data_;
}

const char* MappedMemory::data() const {
	return data_;
}

QString MappedMemory::key() const {
	return file_->fileName();
}

void MappedMemory::resize(quint64 size) {
	lock();
	munmap(data_, length_);
	if (!file_->resize(size)) {
		qFatal("Mapped file resize failed");
	}
	Q_ASSERT(size == file_->size());
	void* data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file_->handle(), 0);
	if ((qint64)data == -1) {
		qFatal(strerror(errno));
	}

	data_ = static_cast<char*>(data);
	length_ = size;
	qDebug() << (quintptr)data_;
	unlock();
}
