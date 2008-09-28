#include "mapped_memory.h"

#include <cstdio>
#include <sys/mman.h>

#include <QSharedMemory>

MappedMemory::MappedMemory() {
	QString key = QCoreApplication::applicationName() + "-" +
	              QString::number(QCoreApplication::applicationPid()) + "-" +
				  QString::number(qrand());
	data_ = new QSharedMemory(key, this);
	data_->create(8*1024*1024);
}

MappedMemory::MappedMemory(const QString& key)
	: key_(key) {
	data_ = new QSharedMemory(key, this);
	data_->attach();
}

MappedMemory::~MappedMemory() {
}

bool MappedMemory::lock() {
	data_.lock();
}

bool MappedMemory::unlock() {
	data_.lock();
}

char* MappedMemory::data() {
	return data_->data();
}

const char* MappedMemory::data() const {
	return data_->data();
}

QString MappedMemory::key() const {
	return data_->key();
}

void MappedMemory::resize(quint64 size) {
	// TODO
}


