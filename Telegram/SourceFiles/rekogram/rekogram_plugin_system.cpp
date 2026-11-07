/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_plugin_system.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kPluginsPref = "reko_plugins_state";

} // namespace

PluginManager &PluginManager::Instance() {
	static PluginManager instance;
	return instance;
}

PluginManager::PluginManager() {
	// Standard built-in Rekogram extension modules
	_plugins = {
		{ "org.rekogram.bookmarks", "Message Bookmarks Extension", "1.0.0", "Rekogram Team", "Сохранение и категоризация сообщений в закладки", true },
		{ "org.rekogram.reminders", "Message Reminders Extension", "1.0.0", "Rekogram Team", "Планировщик локальных напоминаний о сообщениях", true },
		{ "org.rekogram.streamer", "Streamer Shield Mode", "1.0.0", "Rekogram Team", "Маскировка конфиденциальных данных при демонстрации экрана", true },
		{ "org.rekogram.inspector", "Message Inspector Engine", "1.0.0", "Rekogram Team", "Детальный анализ метаданных сообщений", true },
		{ "org.rekogram.quickreplies", "Quick Replies & Snippets", "1.0.0", "Rekogram Team", "Быстрые шаблоны ответов по ключевым фразам", true }
	};
	load();
}

void PluginManager::load() {
	const auto raw = Core::App().settings().readPref<QByteArray>(kPluginsPref, QByteArray());
	if (raw.isEmpty()) return;

	const auto doc = QJsonDocument::fromJson(raw);
	if (!doc.isObject()) return;

	const auto obj = doc.object();
	for (auto &p : _plugins) {
		if (obj.contains(p.id)) {
			p.enabled = obj[p.id].toBool(true);
		}
	}
}

void PluginManager::save() {
	QJsonObject obj;
	for (const auto &p : _plugins) {
		obj[p.id] = p.enabled;
	}
	const auto bytes = QJsonDocument(obj).toJson(QJsonDocument::Compact);
	Core::App().settings().writePref<QByteArray>(kPluginsPref, bytes);
}

std::vector<PluginManifest> PluginManager::plugins() const {
	return _plugins;
}

void PluginManager::setPluginEnabled(const QString &id, bool enabled) {
	for (auto &p : _plugins) {
		if (p.id == id) {
			p.enabled = enabled;
			save();
			break;
		}
	}
}

bool PluginManager::isPluginEnabled(const QString &id) const {
	for (const auto &p : _plugins) {
		if (p.id == id) return p.enabled;
	}
	return false;
}

} // namespace Rekogram
