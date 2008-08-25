#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
	Q_OBJECT
public:
	static Settings* instance();
	
	enum FeedBehaviour {
		Auto = 0,
		ShowInline = 1,
		OpenInBrowser = 2
	};
	
	QString googleUsername() const;
	QString googlePassword() const;
	int behaviour(const QString& feedId) const;
	
	void setGoogleAccount(const QString& username, const QString& password);
	void setBehaviour(const QString& feedId, int behaviour);

signals:
	void googleAccountChanged();

private:
	Settings();
	~Settings() {}
	
	void readBehaviours();
	void writeBehaviours();
	
	static Settings* s_instance;
	QSettings m_settings;
	
	QMap<QString, FeedBehaviour> behaviour_;
	bool dirtyBehaviour_; // hehe
};

#endif
