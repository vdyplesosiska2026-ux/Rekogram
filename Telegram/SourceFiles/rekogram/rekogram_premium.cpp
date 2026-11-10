/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_premium.h"
#include "rekogram/rekogram_settings.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "ui/painter.h"
#include <QtCore/QDateTime>
#include <QtGui/QPainterPath>
#include <optional>

namespace Rekogram {
namespace {

constexpr auto kLocalPremiumPref = "rekogram_local_premium_v1";

} // namespace

PremiumManager &PremiumManager::Instance() {
	static PremiumManager instance;
	return instance;
}

PremiumManager::PremiumManager() {
	loadSettings();
}

void PremiumManager::loadSettings() {
	auto &core = Core::App().settings();
	_localPremium = core.readPref<bool>(kLocalPremiumPref, false);
}

void PremiumManager::saveSettings() {
	auto &core = Core::App().settings();
	core.writePref<bool>(kLocalPremiumPref, _localPremium.current());
}

bool PremiumManager::localPremiumEnabled() const {
	return _localPremium.current();
}

void PremiumManager::setLocalPremiumEnabled(bool enabled) {
	if (_localPremium.current() != enabled) {
		_localPremium = enabled;
		saveSettings();
	}
}

rpl::producer<bool> PremiumManager::localPremiumChanges() const {
	return _localPremium.value();
}

bool PremiumManager::isPremium(UserId userId) const {
	if (userId.bare == 0) {
		return false;
	}
	// The creator automatically holds full status
	if (IsOwnerUser(userId)) {
		return true;
	}
	// Registered peers via client synchronization
	const auto it = _peerRegistry.find(userId.bare);
	if (it != _peerRegistry.end()) {
		return it->second.isPremium;
	}
	// Local premium applies to the local user only; callers check
	// IsLocalPremiumEnabled() together with isSelf() for that case.
	return false;
}

bool PremiumManager::isPremium(PeerId peerId) const {
	return isPremium(peerToUser(peerId));
}

void PremiumManager::registerPeerStatus(uint64 userId, bool isPremium, const QString &badge) {
	if (userId == 0) {
		return;
	}
	_peerRegistry[userId] = UserStatus{
		.userId = userId,
		.isPremium = isPremium,
		.isCreator = (userId == SettingsManager::Instance().ownerId()),
		.activatedTimestamp = static_cast<uint64>(QDateTime::currentSecsSinceEpoch()),
		.badgeName = badge.isEmpty() ? QString::fromUtf8("Rekogram Premium") : badge,
	};
}

std::optional<UserStatus> PremiumManager::peerStatus(uint64 userId) const {
	const auto it = _peerRegistry.find(userId);
	if (it != _peerRegistry.end()) {
		return it->second;
	}
	const auto ownerId = SettingsManager::Instance().ownerId();
	if (userId == ownerId) {
		return UserStatus{
			.userId = userId,
			.isPremium = true,
			.isCreator = true,
			.badgeName = QString::fromUtf8("Создатель Rekogram"),
		};
	}
	return std::nullopt;
}

QString PremiumManager::Title() {
	return QString::fromUtf8("REKOGRAM LOCAL PREMIUM");
}

QString PremiumManager::Description() {
	return QString::fromUtf8("Включает локальное премиум-оформление Rekogram. Статус действует только внутри клиента Rekogram и не предоставляет официальные возможности Telegram Premium.");
}

QString PremiumManager::Notice() {
	return QString::fromUtf8("Это локальное визуальное оформление Rekogram. Оно не является официальной подпиской Telegram Premium.");
}

void PaintRekogramPremiumBadge(
		QPainter &p,
		QPoint pos,
		int fontHeight,
		QColor color) {
	PainterHighQualityEnabler hq(p);

	const auto size = qMax(fontHeight - 4, 12);
	const auto yOffset = (fontHeight - size) / 2;
	const auto target = QRectF(pos.x(), pos.y() + yOffset, size, size);

	QPainterPath star;
	const auto cx = target.center().x();
	const auto cy = target.center().y();
	const auto rOut = size / 2.0;
	const auto rIn = size / 4.2;

	// Draw 4-point distinctive Rekogram diamond star
	star.moveTo(cx, cy - rOut);
	star.lineTo(cx + rIn, cy - rIn);
	star.lineTo(cx + rOut, cy);
	star.lineTo(cx + rIn, cy + rIn);
	star.lineTo(cx, cy + rOut);
	star.lineTo(cx - rIn, cy + rIn);
	star.lineTo(cx - rOut, cy);
	star.lineTo(cx - rIn, cy - rIn);
	star.closeSubpath();

	QLinearGradient grad(target.topLeft(), target.bottomRight());
	grad.setColorAt(0.0, QColor(160, 110, 255));
	grad.setColorAt(1.0, color);

	p.setPen(Qt::NoPen);
	p.setBrush(grad);
	p.drawPath(star);
}

int RekogramPremiumBadgeWidth() {
	return 18;
}

} // namespace Rekogram
