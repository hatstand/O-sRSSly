#ifndef SETTINGS_H
#define SETTINGS_H

#include <QRect>
#include <QSettings>

#include <boost/scoped_ptr.hpp>

#include "keychain.h"

class Settings : public QObject {
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
	
	bool showTrayIcon() const;
	bool startMinimized() const;
	bool checkNew() const;
	int checkNewInterval() const;
	bool showBubble() const;
	
	void setGoogleAccount(const QString& username, const QString& password);
	void setBehaviour(const QString& feedId, int behaviour);
	void setProgressBarStyle(int style);
	void setUnreadOnly(bool unread_only);
	
	void setShowTrayIcon(bool v);
	void setStartMinimized(bool v);
	void setCheckNew(bool v);
	void setCheckNewInterval(int v);
	void setShowBubble(bool v);

	void setGeometry(const QByteArray& geometry);
	QByteArray geometry();

	void commit() { m_settings.sync(); }

signals:
	void googleAccountChanged();
	void progressBarStyleChanged(int v);
	void unreadOnlyChanged(bool v);
	
	void showTrayIconChanged(bool v);
	void startMinimizedChanged(bool v);
	void checkNewChanged(bool v);
	void checkNewIntervalChanged(bool v);
	void showBubbleChanged(bool v);

private:
	Settings();
	~Settings() {}
	
	void readBehaviours();
	void writeBehaviours();
	
	static Settings* s_instance;
	QSettings m_settings;
	
	QMap<QString, FeedBehaviour> behaviour_;
	bool dirtyBehaviour_; // hehe

	boost::scoped_ptr<Keychain> keychain_;
};

#endif
