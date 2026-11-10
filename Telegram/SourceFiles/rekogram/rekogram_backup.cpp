/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_backup.h"

#include "rekogram/rekogram_settings.h"
#include "rekogram/rekogram_streamer_mode.h"
#include "rekogram/rekogram_premium.h"
#include "rekogram/rekogram_plugin_system.h"
#include "rekogram/rekogram_bookmarks.h"
#include "rekogram/rekogram_reminders.h"
#include "rekogram/rekogram_quick_replies.h"
#include "rekogram/rekogram_ai_chat.h"
#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kBackupVersion = 2;

// Pref keys mirrored from the module managers (kept in sync manually).
constexpr auto kBookmarksPref = "reko_bookmarks_data";
constexpr auto kRemindersPref = "reko_reminders_data";
constexpr auto kQuickRepliesPref = "reko_quick_replies_data";
constexpr auto kAIChatHistoryPref = "reko_ai_chat_history";
constexpr auto kPluginsPref = "reko_plugins_state";
constexpr auto kLocalPremiumPref = "rekogram_local_premium_v1";
constexpr auto kStreamerEnabledPref = "reko_streamer_enabled";
constexpr auto kStreamerHidePhonePref = "reko_streamer_hide_phone";
constexpr auto kStreamerHideUserDataPref = "reko_streamer_hide_user_data";
constexpr auto kStreamerMuteNotifPref = "reko_streamer_mute_notif";

void ExportRawJson(
		QJsonObject &modules,
		const char *key,
		const QByteArray &data) {
	const auto text = QString::fromUtf8(data);
	if (!text.isEmpty()) {
		modules[key] = text;
	}
}

bool ImportRawJson(
		const QJsonObject &modules,
		const char *key,
		const char *pref) {
	if (!modules.contains(key)) {
		return false;
	}
	Core::App().settings().writePref<QByteArray>(
		pref,
		modules[key].toString().toUtf8());
	return true;
}

} // namespace

QByteArray BackupManager::ExportSettings() {
	auto &settings = SettingsManager::Instance();
	QJsonObject root;
	root["version"] = kBackupVersion;
	root["app"] = "Rekogram";

	QJsonObject config;
	config["owner_id"] = QString::number(settings.ownerId());
	config["compact_chat_list"] = settings.compactChatList();
	config["chat_avatar_radius"] = settings.chatListAvatarRadius();
	config["experimental_tab"] = settings.experimentalTabEnabled();
	config["dev_mode"] = settings.devMode();
	config["accent_color"] = settings.accentColorHex();
	root["settings"] = config;

	QJsonObject modules;
	auto &core = Core::App().settings();
	ExportRawJson(modules, "bookmarks", core.readPref<QByteArray>(kBookmarksPref, QByteArray()));
	ExportRawJson(modules, "reminders", core.readPref<QByteArray>(kRemindersPref, QByteArray()));
	ExportRawJson(modules, "quick_replies", core.readPref<QByteArray>(kQuickRepliesPref, QByteArray()));
	ExportRawJson(modules, "ai_chat_history", core.readPref<QByteArray>(kAIChatHistoryPref, QByteArray()));
	ExportRawJson(modules, "plugins", core.readPref<QByteArray>(kPluginsPref, QByteArray()));

	QJsonObject streamer;
	streamer["enabled"] = core.readPref<bool>(kStreamerEnabledPref, false);
	streamer["hide_phone"] = core.readPref<bool>(kStreamerHidePhonePref, true);
	streamer["hide_user_data"] = core.readPref<bool>(kStreamerHideUserDataPref, true);
	streamer["mute_notif"] = core.readPref<bool>(kStreamerMuteNotifPref, true);
	modules["streamer"] = streamer;

	QJsonObject premium;
	premium["local_premium"] = core.readPref<bool>(kLocalPremiumPref, false);
	modules["premium"] = premium;

	root["modules"] = modules;

	return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool BackupManager::ImportSettings(const QByteArray &data, QString *outError) {
	QJsonParseError parseError;
	const auto doc = QJsonDocument::fromJson(data, &parseError);
	if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
		if (outError) *outError = QString::fromUtf8("Неверный формат JSON-файла конфигурации.");
		return false;
	}

	const auto root = doc.object();
	if (root["app"].toString() != "Rekogram") {
		if (outError) *outError = QString::fromUtf8("Файл не является резервной копией Rekogram.");
		return false;
	}

	const auto version = root["version"].toInt();
	if (version > kBackupVersion) {
		if (outError) *outError = QString::fromUtf8("Файл создан в более новой версии Rekogram.");
		return false;
	}

	const auto config = root["settings"].toObject();
	auto &settings = SettingsManager::Instance();

	if (config.contains("compact_chat_list")) {
		settings.setCompactChatList(config["compact_chat_list"].toBool());
	}
	if (config.contains("chat_avatar_radius")) {
		settings.setChatListAvatarRadius(config["chat_avatar_radius"].toInt());
	}
	if (config.contains("experimental_tab")) {
		settings.setExperimentalTabEnabled(config["experimental_tab"].toBool());
	}
	if (config.contains("dev_mode")) {
		settings.setDevMode(config["dev_mode"].toBool());
	}
	if (config.contains("accent_color")) {
		settings.setAccentColorHex(config["accent_color"].toString());
	}

	const auto modules = root["modules"].toObject();
	if (!modules.isEmpty()) {
		if (ImportRawJson(modules, "bookmarks", kBookmarksPref)) {
			BookmarksManager::Instance().reload();
		}
		if (ImportRawJson(modules, "reminders", kRemindersPref)) {
			RemindersManager::Instance().reload();
		}
		if (ImportRawJson(modules, "quick_replies", kQuickRepliesPref)) {
			QuickRepliesManager::Instance().reload();
		}
		if (ImportRawJson(modules, "ai_chat_history", kAIChatHistoryPref)) {
			AIChatManager::Instance().reload();
		}
		if (ImportRawJson(modules, "plugins", kPluginsPref)) {
			PluginManager::Instance().reloadAll();
		}

		const auto streamer = modules["streamer"].toObject();
		if (!streamer.isEmpty()) {
			auto &sm = StreamerModeManager::Instance();
			if (streamer.contains("enabled")) sm.setEnabled(streamer["enabled"].toBool());
			if (streamer.contains("hide_phone")) sm.setHidePhone(streamer["hide_phone"].toBool());
			if (streamer.contains("hide_user_data")) sm.setHideUserData(streamer["hide_user_data"].toBool());
			if (streamer.contains("mute_notif")) sm.setMuteNotificationContent(streamer["mute_notif"].toBool());
		}

		const auto premium = modules["premium"].toObject();
		if (premium.contains("local_premium")) {
			PremiumManager::Instance().setLocalPremiumEnabled(premium["local_premium"].toBool());
		}
	}

	return true;
}

} // namespace Rekogram
