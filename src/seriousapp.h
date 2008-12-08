#ifndef SERIOUSAPP_H
#define SERIOUSAPP_H

#include <QApplication>

class SeriousApp : public QApplication {
public:
	SeriousApp(int argc, char** argv);
	virtual void commitData(QSessionManager& session);
	virtual void saveState(QSessionManager& session);
};

#endif
