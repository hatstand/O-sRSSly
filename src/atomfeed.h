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

class Database;
class QIODevice;
class QSqlQuery;

// A single entry from an atom feed (ie. an article).
class AtomEntry
{
public:
	AtomEntry() {}
	AtomEntry(QXmlStreamReader& s);
	AtomEntry(const QSqlQuery& query);
	
	const QString& previewText() const;
	
	// Update struct update_entry when you add new fields.
	QString title;
	QString id;
	QString summary;
	QString content;
	QDateTime date;
	QUrl link;
	bool read;
	bool starred;
	QString source;
	QString author;
	QString shared_by;
	
	void update(Database* db) const;

private:
	void parseSource(QXmlStreamReader& s);
	QString parseAuthor(QXmlStreamReader& s);
	QString previewText_;
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

	AtomFeed() {}
	AtomFeed(const QString& id, Database* db);
	AtomFeed(const QUrl& url, QIODevice* data, Database* db);
	AtomFeed(const QSqlQuery& query, Database* db);
	~AtomFeed();
	
	bool hasError() const { return m_error; }
	
	// Subscription id.
	QString id() const { return m_id; }
	// Subscription title.
	QString title() const { return m_title; }
	// Subscription url.
	QUrl url() const { return m_url; }
	// All the known Atom entries.
	const AtomList& entries() const { return m_entries.get<random>(); }

	const QString& continuation() const { return m_continuation; }

	const QDateTime& updated() const { return m_updated; }
	
	// Copy all the entries from the other AtomFeed into this one.
	// New entries with duplicate ids are ignored.
	// If definitive is set, then old entries that new entries are updated
	// with the changed data.
	void merge(const AtomFeed& other, bool definitive = false);

	// Add a single entry
	void add(const AtomEntry& e, bool definitive = false);

	// Marks the entry as read (local only).
	void setRead(const AtomEntry& e);

	void setStarred(const AtomEntry& e, bool starred);
	
	void saveEntries();
	void updateEntry(const AtomEntry& entry);

	AtomFeed* createAtomFeed(const QSqlQuery& q);

	static const char* kReaderXmlNamespace;
private:
	void parse(QIODevice* device);
	void parseFeed(QXmlStreamReader& s);
	void addEntries(const QSqlQuery& query);
	
	struct update_entry {
		update_entry(const AtomEntry& new_entry) : new_entry_(new_entry) {}

		void operator() (AtomEntry& old_entry) {
			old_entry.title = new_entry_.title;
			old_entry.summary = new_entry_.summary;
			old_entry.content = new_entry_.content;
			old_entry.date = new_entry_.date;
			old_entry.link = new_entry_.link;
			old_entry.read = new_entry_.read;
			old_entry.starred = new_entry_.starred;
			old_entry.source = new_entry_.source;
			old_entry.author = new_entry_.author;
			old_entry.shared_by = new_entry_.shared_by;
		}

		const AtomEntry& new_entry_;
	};

	struct set_read {
		void operator() (AtomEntry& e) {
			e.read = true;
			e.date = QDateTime::currentDateTime();
		}
	};
	
	struct set_starred {
		set_starred(bool starred) : starred_(starred) {}
		void operator() (AtomEntry& e) {
			e.starred = starred_;
			e.date = QDateTime::currentDateTime();
		}

		bool starred_;
	};

	bool m_error;
	
	QString m_id;
	QString m_title;
	QUrl m_url;
	// Magic string which represents to Google where we got up to in downloading entries.
	QString m_continuation;
	AtomEntryList m_entries;
	QDateTime m_updated;

	Database* db_;
};

QDebug operator <<(QDebug dbg, const AtomFeed& f);

#endif
