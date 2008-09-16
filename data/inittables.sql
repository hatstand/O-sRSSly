CREATE TABLE IF NOT EXISTS Tag
(
	id TEXT PRIMARY KEY,
	title TEXT
)

CREATE TABLE IF NOT EXISTS FeedTagMap
(
	tagId TEXT,
	feedId INTEGER
)

CREATE TABLE IF NOT EXISTS Feed
(
	id TEXT,
	title TEXT,
	sortId TEXT
)

CREATE TABLE IF NOT EXISTS Entry
(
	feedId INTEGER,
	title TEXT,
	id TEXT,
	summary TEXT,
	content TEXT,
	date TEXT,
	link TEXT,
	read INTEGER,
	starred INTEGER,
	author TEXT,
	shared_by TEXT
)
