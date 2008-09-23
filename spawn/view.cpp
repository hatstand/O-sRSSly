#include "view.h"
#include "manager.h"
#include "inputevents.pb.h"
#include "child.h"

#include <QEvent>
#include <QtDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QWebView>

namespace Spawn {

View::View(Manager* manager, QWidget* parent)
	: QWidget(parent),
	  manager_(manager),
	  message_view_(NULL)
{
	connect(manager, SIGNAL(destroyed(QObject*)), SLOT(managerDestroyed()));
	
	child_ = manager_->createPage();
	connect(child_, SIGNAL(repaintRequested(const QRect&)), SLOT(repaintRequested(const QRect&)));
	connect(child_, SIGNAL(scrollRequested(int, int, const QRect&)), SLOT(scrollRequested(int, int, const QRect&)));
	connect(child_, SIGNAL(stateChanged(Child::State)), SLOT(childStateChanged(Child::State)));
	connect(child_, SIGNAL(loadFinished(bool)), SIGNAL(loadFinished(bool)));
	connect(child_, SIGNAL(loadProgress(int)), SIGNAL(loadProgress(int)));
	connect(child_, SIGNAL(loadStarted()), SIGNAL(loadStarted()));
	connect(child_, SIGNAL(statusBarMessage(const QString&)), SIGNAL(statusBarMessage(const QString&)));
	connect(child_, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
	connect(child_, SIGNAL(urlChanged(const QUrl&)), SIGNAL(urlChanged(const QUrl&)));
	connect(child_, SIGNAL(linkClicked(const QUrl&)), SIGNAL(linkClicked(const QUrl&)));
	
	QPalette pal = palette();
	pal.setBrush(QPalette::Background, Qt::white);
	setPalette(pal);
	
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	
	setMouseTracking(true);
	setFocusPolicy(Qt::WheelFocus);
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

View::~View() {
	qDebug() << __PRETTY_FUNCTION__;
	if (manager_)
		manager_->destroyPage(child_);
}

void View::setUrl(const QUrl& url) {
	child_->setUrl(url);
}

void View::setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy) {
	child_->setLinkDelegationPolicy(policy);
}

void View::setHtml(const QString& html) {
	child_->setHtml(html);
}

void View::resizeEvent(QResizeEvent*) {
	child_->sendResizeEvent(width(), height());
	
	if (message_view_)
		message_view_->resize(width(), height());
}

void View::mouseMoveEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_MOVE_EVENT, e);
}

void View::mousePressEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_PRESS_EVENT, e);
}

void View::mouseReleaseEvent(QMouseEvent* e) {
	child_->sendMouseEvent(SpawnEvent_Type_MOUSE_RELEASE_EVENT, e);
}

void View::keyPressEvent(QKeyEvent* e) {
	child_->sendKeyEvent(SpawnEvent_Type_KEY_PRESS_EVENT, e);
}

void View::keyReleaseEvent(QKeyEvent* e) {
	child_->sendKeyEvent(SpawnEvent_Type_KEY_RELEASE_EVENT, e);
}

void View::wheelEvent(QWheelEvent* e) {
	child_->sendWheelEvent(e);
}

void View::paintEvent(QPaintEvent* e) {
	QPainter p(this);
	p.setClipRect(e->rect());
	
	switch (child_->state()) {
	case Child::Ready:
		child_->paint(p, e->rect());
		break;
	
	case Child::Starting:
		p.fillRect(e->rect(), palette().brush(QPalette::Background));
		break;
	
	default:
		break;
	}
}

void View::repaintRequested(const QRect& rect) {
	if (rect.isNull())
		update();
	else
		update(rect);
}

void View::childStateChanged(Child::State state) {
	qDebug() << __PRETTY_FUNCTION__;
	if (state == Child::Error) {
		if (!message_view_) {
			message_view_ = new QWebView(this);
			connect(message_view_, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
			connect(message_view_->page(), SIGNAL(linkClicked(const QUrl&)), SLOT(messageLinkClicked(const QUrl&)));
			message_view_->resize(width(), height());
			message_view_->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
		}
		
		message_view_->setUrl(QUrl("qrc:/spawn/error.html"));
		message_view_->show();
	} else {
		if (message_view_)
			message_view_->hide();
	}
}

void View::messageLinkClicked(const QUrl& url) {
	if (url.scheme() != "spawn")
		return;
	
	if (url.path() == "/restart") {
		manager_->restartPage(child_);
		child_->sendResizeEvent(width(), height());
	}
}

QSize View::sizeHint() const {
	return QSize(800, 600);
}

void View::managerDestroyed() {
	manager_ = NULL;
	child_ = NULL;
}

void View::scrollRequested(int dx, int dy, const QRect& rectToScroll) {
	// Scroll the existing area
	scroll(dx, dy, rectToScroll);
	
	// Qt will repaint the exposed areas automatically
}


}
