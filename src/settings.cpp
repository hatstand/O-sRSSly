#include "keychain.h"
#include "settings.h"

Settings* Settings::s_instance = NULL;

Settings* Settings::instance() {
	if (!s_instance)
		s_instance = new Settings;
	return s_instance;
	
}

Settings::Settings()
	: dirtyBehaviour_(true)
{
}

QString Settings::googleUsername() const {
	return m_settings.value("google/username").toString();
}

QString Settings::googlePassword() const {
	QString user = m_settings.value("google/username").toString();
	QString password = Keychain::getPassword(user);
	
	return password;
}

void Settings::setGoogleAccount(const QString& username, const QString& password) {
	QString oldUsername(googleUsername());
	QString oldPassword(googlePassword());
	
	m_settings.setValue("google/username", username);

	Keychain::setPassword(username, password);
	
	if (username != oldUsername || password != oldPassword)
		emit googleAccountChanged();
}

void Settings::readBehaviours() {
	if (!dirtyBehaviour_)
		return;
	
	behaviour_.clear();
	
	int size = m_settings.beginReadArray("feedbehaviour");
	for (int i=0 ; i<size ; ++i) {
		m_settings.setArrayIndex(i);
		behaviour_[m_settings.value("id").toString()] = (FeedBehaviour) m_settings.value("behaviour").toInt();
	}
	m_settings.endArray();
	
	dirtyBehaviour_ = false;
}

void Settings::writeBehaviours() {
	m_settings.beginWriteArray("feedbehaviour", behaviour_.count());
	
	QMap<QString, FeedBehaviour>::const_iterator it = behaviour_.constBegin();
	int i = 0;
	while (it != behaviour_.constEnd()) {
		m_settings.setArrayIndex(i++);
		m_settings.setValue("id", it.key());
		m_settings.setValue("behaviour", it.value());
		++it;
	}
	m_settings.endArray();
}

int Settings::behaviour(const QString& feedId) const {
	if (dirtyBehaviour_)
		const_cast<Settings*>(this)->readBehaviours();
	
	if (behaviour_.contains(feedId))
		return behaviour_[feedId];
	return 0;
}

void Settings::setBehaviour(const QString& feedId, int behaviour) {
	behaviour_[feedId] = (FeedBehaviour) behaviour;
	writeBehaviours();
}
