#include "child.h"
#include "manager.h"

#include <QDebug>

namespace Spawn {

Child::Child(Manager* manager, quint64 id)
	: QObject(manager),
	  starting_process_(NULL),
	  manager_(manager),
	  id_(id),
	  ready_(false)
{
	qDebug() << __PRETTY_FUNCTION__;
}

void Child::setReady() {
	qDebug() << __PRETTY_FUNCTION__;
	ready_ = true;
	starting_process_ = NULL;
	emit ready();
}

}
