/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_quick_replies.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kQuickRepliesPref = "reko_quick_replies_data";

} // namespace

QuickRepliesManager &QuickRepliesManager::Instance() {
	static QuickRepliesManager instance;
	return instance;
}

QuickRepliesManager::QuickRepliesManager() {
	load();
}

void QuickRepliesManager::load() {
	_items.clear();
	const auto raw = Core::App().settings().readPref<QByteArray>(kQuickRepliesPref, QByteArray());
	if (raw.isEmpty()) {
		_items = {
			{ "!hi", "Привет! Сейчас немного занят, отвечу чуть позже." },
			{ "!ok", "Принято, спасибо!" },
			{ "!thx", "Благодарю за информацию!" }
		};
		return;
	}

	const auto doc = QJsonDocument::fromJson(raw);
	if (!doc.isArray()) return;

	for (const auto &val : doc.array()) {
		if (!val.isObject()) continue;
		const auto obj = val.toObject();
		_items.push_back({ obj["key"].toString(), obj["val"].toString() });
	}
}

void QuickRepliesManager::save() {
	QJsonArray arr;
	for (const auto &item : _items) {
		QJsonObject obj;
		obj["key"] = item.keyword;
		obj["val"] = item.text;
		arr.append(obj);
	}
	const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Compact);
	Core::App().settings().writePref<QByteArray>(kQuickRepliesPref, bytes);
}

void QuickRepliesManager::addTemplate(const QString &keyword, const QString &text) {
	removeTemplate(keyword);
	_items.push_back({ keyword, text });
	save();
}

void QuickRepliesManager::removeTemplate(const QString &keyword) {
	auto it = std::remove_if(_items.begin(), _items.end(), [&](const QuickReplyItem &item) {
		return item.keyword.compare(keyword, Qt::CaseInsensitive) == 0;
	});
	if (it != _items.end()) {
		_items.erase(it, _items.end());
		save();
	}
}

std::vector<QuickReplyItem> QuickRepliesManager::list() const {
	return _items;
}

QString QuickRepliesManager::findText(const QString &keyword) const {
	for (const auto &item : _items) {
		if (item.keyword.compare(keyword, Qt::CaseInsensitive) == 0) {
			return item.text;
		}
	}
	return QString();
}

} // namespace Rekogram
