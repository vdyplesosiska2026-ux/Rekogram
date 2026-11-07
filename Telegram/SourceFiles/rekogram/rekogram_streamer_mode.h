/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <rpl/variable.h>

namespace Rekogram {

class StreamerModeManager final {
public:
	static StreamerModeManager &Instance();

	[[nodiscard]] bool enabled() const;
	void setEnabled(bool enabled);
	[[nodiscard]] rpl::producer<bool> enabledChanges() const;

	[[nodiscard]] bool hidePhone() const;
	void setHidePhone(bool hide);

	[[nodiscard]] bool hideUserData() const;
	void setHideUserData(bool hide);

	[[nodiscard]] bool muteNotificationContent() const;
	void setMuteNotificationContent(bool mute);

	[[nodiscard]] QString maskString(const QString &original) const;

private:
	StreamerModeManager();
	void load();
	void save();

	rpl::variable<bool> _enabled = false;
	bool _hidePhone = true;
	bool _hideUserData = true;
	bool _muteNotificationContent = true;
};

[[nodiscard]] inline bool IsStreamerModeActive() {
	return StreamerModeManager::Instance().enabled();
}

} // namespace Rekogram
