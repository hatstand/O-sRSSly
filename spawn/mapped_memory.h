#ifndef MAPPED_MEMORY_H
#define MAPPED_MEMORY_H

#include <QString>
#include <QTemporaryFile>

class QSharedMemory;

class MappedMemory {
public:
	MappedMemory();
	MappedMemory(const QString& key);
	~MappedMemory();
	char* data();
	const char* data() const;
	QString key() const;
	bool lock();
	bool unlock();
	void resize(quint64 size);

private:
#ifdef Q_OS_UNIX
	void init(int fd);

	QTemporaryFile tmp_file_;
	QString key_;
	char* data_;
	quint64 length_;
#else
	QSharedMemory* data_;
#endif
};

#endif
