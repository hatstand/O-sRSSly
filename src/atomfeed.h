#ifndef ATOMFEED_H
#define ATOMFEED_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <QXmlStreamReader> // Do not change to class QXmlStreamReader (gcc 4.0.1)

#include <set>

class QIODevice;

class AtomEntry
{
public:
	AtomEntry(QXmlStreamReader& s);
	
	QString title;
	QString id;
	QString summary;
	QString content;
	QDateTime date;
	bool read;
};

struct AtomCompare {
	bool operator() (const AtomEntry& a, const AtomEntry& b) const {
		if (a.date == b.date)
			return a.id < b.id;

		return a.date < b.date;
	}
};

QDebug operator <<(QDebug dbg, const AtomEntry& e);

class AtomFeed : public QAbstractTableModel
{
public:
	AtomFeed();
	AtomFeed(const QString& fileName);
	AtomFeed(QIODevice* device);
	~AtomFeed();
	
	bool hasError() const { return m_error; }
	
	QString id() const { return m_id; }
	QString title() const { return m_title; }
	const std::set<AtomEntry, AtomCompare>& entries() const { return m_entries; }

	void merge(const AtomFeed& other);

	// QAbstractTableModel
	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual int rowCount(const QModelIndex& parent) const;

private:
	void init();
	void parse(QIODevice* device);
	void parseFeed(QXmlStreamReader& s);
	
	bool m_error;
	
	QString m_id;
	QString m_title;
	std::set<AtomEntry, AtomCompare> m_entries;
};

QDebug operator <<(QDebug dbg, const AtomFeed& f);

#endif
