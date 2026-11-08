/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "data/data_peer_id.h"
#include <rpl/producer.h>
#include <rpl/variable.h>
#include <QtGui/QPainter>
#include <base/flat_map.h>

namespace Rekogram {

struct UserStatus {
	uint64 userId = 0;
	bool isPremium = false;
	bool isCreator = false;
	uint64 activatedTimestamp = 0;
	QString badgeName;
};

class PremiumManager final {
public:
	static PremiumManager &Instance();

	[[nodiscard]] bool localPremiumEnabled() const;
	void setLocalPremiumEnabled(bool enabled);
	[[nodiscard]] rpl::producer<bool> localPremiumChanges() const;

	[[nodiscard]] bool isPremium(UserId userId) const;
	[[nodiscard]] bool isPremium(PeerId peerId) const;

	void registerPeerStatus(uint64 userId, bool isPremium, const QString &badge = QString());
	[[nodiscard]] std::optional<UserStatus> peerStatus(uint64 userId) const;

	[[nodiscard]] static QString Title();
	[[nodiscard]] static QString Description();
	[[nodiscard]] static QString Notice();

private:
	PremiumManager();
	void loadSettings();
	void saveSettings();

	rpl::variable<bool> _localPremium = false;
	base::flat_map<uint64, UserStatus> _peerRegistry;
};

[[nodiscard]] inline bool IsLocalPremiumEnabled() {
	return PremiumManager::Instance().localPremiumEnabled();
}

[[nodiscard]] inline bool IsUserRekogramPremium(UserId userId) {
	return PremiumManager::Instance().isPremium(userId);
}

[[nodiscard]] inline bool IsUserRekogramPremium(PeerId peerId) {
	return PremiumManager::Instance().isPremium(peerId);
}

void PaintRekogramPremiumBadge(
	QPainter &p,
	QPoint pos,
	int fontHeight,
	QColor color = QColor(140, 90, 240));

inline void PaintRekogramPremiumBadge(
	QPainter &p,
	QPoint pos,
	QColor color = QColor(140, 90, 240)) {
	PaintRekogramPremiumBadge(p, pos, 16, color);
}

[[nodiscard]] int RekogramPremiumBadgeWidth();

} // namespace Rekogram
