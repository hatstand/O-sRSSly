#include "seriousapp.h"

#include "settings.h"

#include <QWidget>

SeriousApp::SeriousApp(int argc, char** argv)
	: QApplication(argc, argv) {
}

void SeriousApp::commitData(QSessionManager& session) {
	Settings::instance()->commit();
}

void SeriousApp::saveState(QSessionManager& session) {
	Settings* settings = Settings::instance();
	settings->setGeometry(topLevelWidgets()[0]->geometry());
}
