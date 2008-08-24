#include "settings.h"

Settings* Settings::s_instance = NULL;

Settings* Settings::instance()
{
	if (!s_instance)
		s_instance = new Settings;
	return s_instance;
	
}

Settings::Settings()
{
}

QString Settings::googleUsername() const
{
	return m_settings.value("google/username").toString();
}

QString Settings::googlePassword() const
{
	return m_settings.value("google/password").toString();
}

void Settings::setGoogleAccount(const QString& username, const QString& password)
{
	QString oldUsername(googleUsername());
	QString oldPassword(googlePassword());
	
	m_settings.setValue("google/username", username);
	m_settings.setValue("google/password", password);
	
	if (username != oldUsername || password != oldPassword)
		emit googleAccountChanged();
}
