/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_badge.h"

#include <QtGui/QPainterPath>

namespace Rekogram {

constexpr int kBadgeWidth = 18;
constexpr int kBadgeHeight = 14;

int CreatorBadgeWidth() {
	return kBadgeWidth + 4; // Width + right margin
}

void PaintCreatorBadge(
		QPainter &p,
		const QPoint &position,
		int height,
		const QColor &accentColor) {
	p.save();
	p.setRenderHint(QPainter::Antialiasing);

	const auto yOffset = position.y() + (height - kBadgeHeight) / 2;
	const auto xOffset = position.x();

	// Draw distinctive Creator Crown / Diamond emblem
	QPainterPath path;
	path.moveTo(xOffset + 2, yOffset + 12);
	path.lineTo(xOffset + 16, yOffset + 12);
	path.lineTo(xOffset + 14, yOffset + 4);
	path.lineTo(xOffset + 9, yOffset + 8);
	path.lineTo(xOffset + 4, yOffset + 4);
	path.closeSubpath();

	QColor badgeColor = accentColor.isValid() ? accentColor : QColor(0x73, 0x62, 0xF2);
	p.setBrush(badgeColor);
	p.setPen(Qt::NoPen);
	p.drawPath(path);

	// Top sparkles/points
	p.setBrush(QColor(0xFF, 0xD7, 0x00)); // Gold crest accents
	p.drawEllipse(QPointF(xOffset + 4, yOffset + 3), 1.5, 1.5);
	p.drawEllipse(QPointF(xOffset + 9, yOffset + 2), 1.8, 1.8);
	p.drawEllipse(QPointF(xOffset + 14, yOffset + 3), 1.5, 1.5);

	p.restore();
}

} // namespace Rekogram
