CREATE TABLE IF NOT EXISTS feeds
(
	url TEXT,
	id TEXT,
	title TEXT
)

CREATE TABLE IF NOT EXISTS entries
(
	title TEXT,
	id TEXT,
	summary TEXT,
	content TEXT,
	date INTEGER,
	read INTEGER
)