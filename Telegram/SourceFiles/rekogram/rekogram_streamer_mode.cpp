/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_streamer_mode.h"

#include "core/application.h"
#include "core/core_settings.h"

namespace Rekogram {
namespace {

constexpr auto kStreamerModeEnabledPref = "reko_streamer_enabled";
constexpr auto kStreamerHidePhonePref = "reko_streamer_hide_phone";
constexpr auto kStreamerHideUserDataPref = "reko_streamer_hide_user_data";
constexpr auto kStreamerMuteNotificationPref = "reko_streamer_mute_notif";

} // namespace

StreamerModeManager &StreamerModeManager::Instance() {
	static StreamerModeManager instance;
	return instance;
}

StreamerModeManager::StreamerModeManager() {
	load();
}

void StreamerModeManager::load() {
	auto &core = Core::App().settings();
	_enabled = core.readPref<bool>(kStreamerModeEnabledPref, false);
	_hidePhone = core.readPref<bool>(kStreamerHidePhonePref, true);
	_hideUserData = core.readPref<bool>(kStreamerHideUserDataPref, true);
	_muteNotificationContent = core.readPref<bool>(kStreamerMuteNotificationPref, true);
}

void StreamerModeManager::save() {
	auto &core = Core::App().settings();
	core.writePref<bool>(kStreamerModeEnabledPref, _enabled.current());
	core.writePref<bool>(kStreamerHidePhonePref, _hidePhone);
	core.writePref<bool>(kStreamerHideUserDataPref, _hideUserData);
	core.writePref<bool>(kStreamerMuteNotificationPref, _muteNotificationContent);
}

bool StreamerModeManager::enabled() const {
	return _enabled.current();
}

void StreamerModeManager::setEnabled(bool enabled) {
	if (_enabled.current() != enabled) {
		_enabled = enabled;
		save();
	}
}

rpl::producer<bool> StreamerModeManager::enabledChanges() const {
	return _enabled.changes();
}

bool StreamerModeManager::hidePhone() const {
	return _hidePhone;
}

void StreamerModeManager::setHidePhone(bool hide) {
	if (_hidePhone != hide) {
		_hidePhone = hide;
		save();
	}
}

bool StreamerModeManager::hideUserData() const {
	return _hideUserData;
}

void StreamerModeManager::setHideUserData(bool hide) {
	if (_hideUserData != hide) {
		_hideUserData = hide;
		save();
	}
}

bool StreamerModeManager::muteNotificationContent() const {
	return _muteNotificationContent;
}

void StreamerModeManager::setMuteNotificationContent(bool mute) {
	if (_muteNotificationContent != mute) {
		_muteNotificationContent = mute;
		save();
	}
}

QString StreamerModeManager::maskString(const QString &original) const {
	if (!_enabled.current() || original.isEmpty()) {
		return original;
	}
	return QString(original.size(), QChar('*'));
}

} // namespace Rekogram
