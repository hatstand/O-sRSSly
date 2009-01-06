#include "seriousapp.h"

#include "settings.h"

#include <QIcon>
#include <QPainter>
#include <QWidget>

SeriousApp::SeriousApp(int argc, char** argv)
	: QApplication(argc, argv),
	  original_image_(":/logo.png") {
}

void SeriousApp::commitData(QSessionManager& session) {
	Settings::instance()->commit();
}

void SeriousApp::saveState(QSessionManager& session) {
}

void SeriousApp::generateImage(const QString& s) {
	QImage result(original_image_.size(), QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&result);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(result.rect(), Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(0, 0, original_image_);
	painter.setOpacity(1.0);

	int default_font_size = 200;
	QFont font("Helvetica", default_font_size);
	font.setBold(true);
	QFontMetrics metrics(font);
	while (metrics.width(s) > result.width() && metrics.height() > result.height()) {
		font = QFont("Helvetica", --default_font_size);
		font.setBold(true);
	}
	QSize text_size(metrics.width(s), metrics.height());

	painter.setFont(font);
	painter.setPen(Qt::white);
	painter.drawText(
		QRectF(QPointF(0.0, 0.0), original_image_.size()),
		Qt::AlignCenter,
		s);
	painter.end();

	icon_ = QPixmap::fromImage(result);
}

void SeriousApp::setUnreadItems(int n) {
#ifdef Q_OS_DARWIN
	generateImage(QString::number(n));
	setWindowIcon(QIcon(icon_));
#endif
}
