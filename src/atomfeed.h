#ifndef ATOMFEED_H
#define ATOMFEED_H

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QXmlStreamReader> // Do not change to class QXmlStreamReader (gcc 4.0.1)
#include <QUrl>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>

class QIODevice;
class QSqlQuery;

// A single entry from an atom feed (ie. an article).
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

// Hash function for boost::multi_index
std::size_t hash_value(const QString& s);

struct AtomCompare {
	bool operator() (const AtomEntry& a, const AtomEntry& b) const {
		if (a.date == b.date)
			return a.id < b.id;

		return a.date < b.date;
	}
};

QDebug operator <<(QDebug dbg, const AtomEntry& e);

// Contains all the known atom entries for a feed.
class AtomFeed
{
public:
	// multi_index tags.
	struct hash{};
	struct random{};

	// Guarantees uniqueness by AtomEntry::id
	// O(log n) lookup through hash
	// O(1) random access.
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
	AtomFeed(const QUrl& url, QIODevice* data);
	~AtomFeed();
	
	static QList<AtomFeed*> loadAll();
	
	bool hasError() const { return m_error; }
	
	// Subscription id.
	QString id() const { return m_id; }
	// Subscription title.
	QString title() const { return m_title; }
	// Subscription url.
	QUrl url() const { return m_url; }
	// All the known Atom entries.
	const AtomList& entries() const { return m_entries.get<random>(); }
	
	void load(const QString& id);
	void save() const;

	// Copy all the entries from the other AtomFeed into this one.
	// New entries with duplicate ids are ignored.
	void merge(const AtomFeed& other);

	// Marks the entry as read (local only).
	void setRead(const AtomEntry& e);

private:
	void init();
	void load(QSqlQuery& query);
	void parse(QIODevice* device);
	void parseFeed(QXmlStreamReader& s);
	
	bool m_error;
	
	qint64 m_dbId;
	QString m_id;
	QString m_title;
	QUrl m_url;
	AtomEntryList m_entries;
};

QDebug operator <<(QDebug dbg, const AtomFeed& f);

#endif
