/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "settings/settings_type.h"

namespace Window {
class SessionController;
} // namespace Window

namespace Settings {

[[nodiscard]] Type RekogramId();
[[nodiscard]] Type RekogramGeneralId();
[[nodiscard]] Type RekogramAppearanceId();
[[nodiscard]] Type RekogramPrivacyId();
[[nodiscard]] Type RekogramChatsId();
[[nodiscard]] Type RekogramMessagesId();
[[nodiscard]] Type RekogramAIId();
[[nodiscard]] Type RekogramPluginsId();
[[nodiscard]] Type RekogramAdvancedId();
[[nodiscard]] Type RekogramPremiumId();
[[nodiscard]] Type RekogramAboutId();

} // namespace Settings
