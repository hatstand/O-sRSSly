CREATE TABLE IF NOT EXISTS Tag
(
	id TEXT,
	title TEXT
)

CREATE TABLE IF NOT EXISTS FeedTagMap
(
	tagId TEXT,
	feedId INTEGER
)

CREATE TABLE IF NOT EXISTS Feed
(
	subscriptionId TEXT,
	subscriptionTitle TEXT,
	subscriptionSortId TEXT,
	feedUrl TEXT,
	feedTitle TEXT
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
	read INTEGER
)