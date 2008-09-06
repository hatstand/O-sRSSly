#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
	Q_OBJECT
public:
	static Settings* instance();
	
	enum FeedBehaviour {
		Behaviour_Auto = 0,
		Behaviour_ShowInline = 1,
		Behaviour_OpenInBrowser = 2,
		Behaviour_Webclip = 3
	};
	
	enum ProgressBarStyle {
		ProgressBar_Normal = 0,
		ProgressBar_Longcat = 1,
		ProgressBar_Tacgnol = 2
	};
	
	QString googleUsername() const;
	QString googlePassword() const;
	int behaviour(const QString& feedId) const;
	int progressBarStyle() const;
	bool unreadOnly() const;
	
	void setGoogleAccount(const QString& username, const QString& password);
	void setBehaviour(const QString& feedId, int behaviour);
	void setProgressBarStyle(int style);
	void setUnreadOnly(bool unread_only);

signals:
	void googleAccountChanged();
	void progressBarStyleChanged();

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
