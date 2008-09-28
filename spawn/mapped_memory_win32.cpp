#include "mapped_memory.h"

#include <QCoreApplication>
#include <QSharedMemory>

MappedMemory::MappedMemory() {
	QString key = QCoreApplication::applicationName() + "-" +
	              QString::number(QCoreApplication::applicationPid()) + "-" +
				  QString::number(qrand());
	data_.reset(new QSharedMemory(key));
	data_->create(8*1024*1024);
}

MappedMemory::MappedMemory(const QString& key)
	: data_(new QSharedMemory(key)) {
	data_->attach();
}

MappedMemory::~MappedMemory() {
}

bool MappedMemory::lock() {
	return data_->lock();
}

bool MappedMemory::unlock() {
	return data_->unlock();
}

char* MappedMemory::data() {
	return static_cast<char*>(data_->data());
}

const char* MappedMemory::data() const {
	return static_cast<const char*>(data_->data());
}

QString MappedMemory::key() const {
	return data_->key();
}

void MappedMemory::resize(quint64 size) {
	// TODO
}


