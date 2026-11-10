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

namespace Rekogram {

// Creator / Owner Telegram ID:
inline constexpr uint64 kDefaultOwnerUserId = 8801314309ULL;

struct Config {
	uint64 ownerUserId = kDefaultOwnerUserId;
	bool compactChatList = false;
	int chatListAvatarRadius = 100; // percent: 100% = circle, 30% = rounded rect
	bool experimentalTabEnabled = true;
	bool devMode = false;
	QString accentColorHex;
};

class SettingsManager final {
public:
	static SettingsManager &Instance();

	[[nodiscard]] bool isOwner(UserId userId) const;
	[[nodiscard]] bool isOwner(PeerId peerId) const;

	[[nodiscard]] uint64 ownerId() const;
	void setOwnerId(uint64 id);

	[[nodiscard]] bool compactChatList() const;
	void setCompactChatList(bool enabled);
	[[nodiscard]] rpl::producer<bool> compactChatListChanges() const;

	[[nodiscard]] int chatListAvatarRadius() const;
	void setChatListAvatarRadius(int radius);

	[[nodiscard]] int avatarRadius() const { return chatListAvatarRadius(); }
	void setAvatarRadius(int radius) { setChatListAvatarRadius(radius); }

	[[nodiscard]] bool showCreatorBadge() const { return _showCreatorBadge; }
	void setShowCreatorBadge(bool enabled) { _showCreatorBadge = enabled; }

	[[nodiscard]] QString creatorRole() const;
	void setCreatorRole(const QString &role);
	[[nodiscard]] QString nextCreatorRole();

	[[nodiscard]] bool experimentalTabEnabled() const;
	void setExperimentalTabEnabled(bool enabled);

	[[nodiscard]] bool devMode() const;
	void setDevMode(bool enabled);

	[[nodiscard]] QString accentColorHex() const;
	void setAccentColorHex(const QString &hex);

	void resetToDefaults();

private:
	SettingsManager();
	void loadSettings();
	void saveSettings();

	uint64 _ownerId = kDefaultOwnerUserId;
	rpl::variable<bool> _compactChatList = false;
	int _chatListAvatarRadius = 100;
	bool _showCreatorBadge = true;
	QString _creatorRole;
	bool _experimentalTabEnabled = true;
	bool _devMode = false;
	QString _accentColorHex;
};

[[nodiscard]] inline bool IsOwnerUser(UserId userId) {
	return SettingsManager::Instance().isOwner(userId);
}

[[nodiscard]] inline bool IsOwnerUser(PeerId peerId) {
	return SettingsManager::Instance().isOwner(peerId);
}

[[nodiscard]] inline bool IsOwnerPeer(PeerId peerId) {
	return SettingsManager::Instance().isOwner(peerId);
}

} // namespace Rekogram
