/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_settings.h"

#include "core/application.h"
#include "core/core_settings.h"

namespace Rekogram {
namespace {

constexpr auto kOwnerIdPref = "reko_owner_id";
constexpr auto kCompactChatListPref = "reko_compact_chat_list";
constexpr auto kChatListAvatarRadiusPref = "reko_chat_avatar_radius";
constexpr auto kExperimentalTabPref = "reko_experimental_tab";
constexpr auto kDevModePref = "reko_dev_mode";
constexpr auto kAccentColorPref = "reko_accent_color";

} // namespace

SettingsManager &SettingsManager::Instance() {
	static SettingsManager instance;
	return instance;
}

SettingsManager::SettingsManager() {
	loadSettings();
}

void SettingsManager::loadSettings() {
	auto &core = Core::App().settings();
	_ownerId = static_cast<uint64>(core.readPref<int>(kOwnerIdPref, static_cast<int>(kDefaultOwnerUserId)));
	_compactChatList = core.readPref<bool>(kCompactChatListPref, false);
	_chatListAvatarRadius = core.readPref<int>(kChatListAvatarRadiusPref, 100);
	_experimentalTabEnabled = core.readPref<bool>(kExperimentalTabPref, true);
	_devMode = core.readPref<bool>(kDevModePref, false);
	const auto accentBytes = core.readPref<QByteArray>(kAccentColorPref, QByteArray());
	_accentColorHex = QString::fromUtf8(accentBytes);
}

void SettingsManager::saveSettings() {
	auto &core = Core::App().settings();
	core.writePref<int>(kOwnerIdPref, static_cast<int>(_ownerId));
	core.writePref<bool>(kCompactChatListPref, _compactChatList.current());
	core.writePref<int>(kChatListAvatarRadiusPref, _chatListAvatarRadius);
	core.writePref<bool>(kExperimentalTabPref, _experimentalTabEnabled);
	core.writePref<bool>(kDevModePref, _devMode);
	core.writePref<QByteArray>(kAccentColorPref, _accentColorHex.toUtf8());
}

bool SettingsManager::isOwner(UserId userId) const {
	if (_ownerId == 0) {
		return false;
	}
	return userId.bare == _ownerId;
}

bool SettingsManager::isOwner(PeerId peerId) const {
	if (_ownerId == 0) {
		return false;
	}
	return peerToUser(peerId).bare == _ownerId;
}

uint64 SettingsManager::ownerId() const {
	return _ownerId;
}

void SettingsManager::setOwnerId(uint64 id) {
	if (_ownerId != id) {
		_ownerId = id;
		saveSettings();
	}
}

bool SettingsManager::compactChatList() const {
	return _compactChatList.current();
}

void SettingsManager::setCompactChatList(bool enabled) {
	if (_compactChatList.current() != enabled) {
		_compactChatList = enabled;
		saveSettings();
	}
}

rpl::producer<bool> SettingsManager::compactChatListChanges() const {
	return _compactChatList.changes();
}

int SettingsManager::chatListAvatarRadius() const {
	return _chatListAvatarRadius;
}

void SettingsManager::setChatListAvatarRadius(int radius) {
	if (_chatListAvatarRadius != radius) {
		_chatListAvatarRadius = radius;
		saveSettings();
	}
}

bool SettingsManager::experimentalTabEnabled() const {
	return _experimentalTabEnabled;
}

void SettingsManager::setExperimentalTabEnabled(bool enabled) {
	if (_experimentalTabEnabled != enabled) {
		_experimentalTabEnabled = enabled;
		saveSettings();
	}
}

bool SettingsManager::devMode() const {
	return _devMode;
}

void SettingsManager::setDevMode(bool enabled) {
	if (_devMode != enabled) {
		_devMode = enabled;
		saveSettings();
	}
}

QString SettingsManager::accentColorHex() const {
	return _accentColorHex;
}

void SettingsManager::setAccentColorHex(const QString &hex) {
	if (_accentColorHex != hex) {
		_accentColorHex = hex;
		saveSettings();
	}
}

void SettingsManager::resetToDefaults() {
	_ownerId = kDefaultOwnerUserId;
	_compactChatList = false;
	_chatListAvatarRadius = 100;
	_experimentalTabEnabled = true;
	_devMode = false;
	_accentColorHex.clear();
	saveSettings();
}

} // namespace Rekogram
