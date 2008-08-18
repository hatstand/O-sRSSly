#ifndef ATOMFEED_H
#define ATOMFEED_H

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QXmlStreamReader> // Do not change to class QXmlStreamReader (gcc 4.0.1)

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>

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

std::size_t hash_value(const QString& s);

struct AtomCompare {
	bool operator() (const AtomEntry& a, const AtomEntry& b) const {
		if (a.date == b.date)
			return a.id < b.id;

		return a.date < b.date;
	}
};

QDebug operator <<(QDebug dbg, const AtomEntry& e);

class AtomFeed
{
public:
	struct hash{};
	struct random{};

	typedef boost::multi_index_container<
		AtomEntry,
		boost::multi_index::indexed_by<
			boost::multi_index::hashed_unique<
				boost::multi_index::tag<hash>,
				boost::multi_index::member<AtomEntry, QString, &AtomEntry::id>,
				boost::hash<QString>
			>,
			boost::multi_index::random_access<
				boost::multi_index::tag<random>
			>
		>
	> AtomEntryList;

	typedef AtomEntryList::index<hash>::type AtomEntries;
	typedef AtomEntryList::index<random>::type AtomList;

	AtomFeed();
	AtomFeed(const QString& fileName);
	AtomFeed(QIODevice* device);
	~AtomFeed();
	
	bool hasError() const { return m_error; }
	
	QString id() const { return m_id; }
	QString title() const { return m_title; }
	const AtomList& entries() const { return m_entries.get<random>(); }

	void merge(const AtomFeed& other);

	void setRead(const AtomEntry& e);

private:
	void init();
	void parse(QIODevice* device);
	void parseFeed(QXmlStreamReader& s);
	
	bool m_error;
	
	QString m_id;
	QString m_title;
	AtomEntryList m_entries;
};

QDebug operator <<(QDebug dbg, const AtomFeed& f);

#endif
