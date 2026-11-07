/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_backup.h"

#include "rekogram/rekogram_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Rekogram {

constexpr auto kBackupVersion = 1;

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

	return true;
}

} // namespace Rekogram
