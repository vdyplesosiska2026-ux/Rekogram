/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtGui/QPainter>
#include <QtCore/QRect>
#include <QtCore/QPoint>

namespace Rekogram {

// Centralized drawing helper for the Rekogram Creator Badge.
// Renders an elegant symbol to the left of the user's name.
void PaintCreatorBadge(
	QPainter &p,
	const QPoint &position,
	int height,
	const QColor &accentColor);

int CreatorBadgeWidth();

} // namespace Rekogram
