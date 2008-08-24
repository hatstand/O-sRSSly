#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QObject
{
	Q_OBJECT
public:
	static Settings* instance();
	
	QString googleUsername() const;
	QString googlePassword() const;
	
	void setGoogleAccount(const QString& username, const QString& password);

signals:
	void googleAccountChanged();

private:
	Settings();
	~Settings() {}
	
	static Settings* s_instance;
	QSettings m_settings;
};

#endif
