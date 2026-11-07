/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_bookmarks.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kBookmarksPref = "reko_bookmarks_data";

} // namespace

BookmarksManager &BookmarksManager::Instance() {
	static BookmarksManager instance;
	return instance;
}

BookmarksManager::BookmarksManager() {
	load();
}

void BookmarksManager::load() {
	_items.clear();
	const auto raw = Core::App().settings().readPref<QByteArray>(kBookmarksPref, QByteArray());
	if (raw.isEmpty()) {
		return;
	}

	const auto doc = QJsonDocument::fromJson(raw);
	if (!doc.isArray()) {
		return;
	}

	const auto arr = doc.array();
	_items.reserve(arr.size());
	for (const auto &val : arr) {
		if (!val.isObject()) continue;
		const auto obj = val.toObject();
		BookmarkItem item;
		item.msgId.peer = PeerId(obj["peer"].toString().toULongLong());
		item.msgId.msg = MsgId(obj["msg"].toString().toLongLong());
		item.previewText = obj["text"].toString();
		item.note = obj["note"].toString();
		item.category = obj["cat"].toString();
		item.savedAt = QDateTime::fromString(obj["date"].toString(), Qt::ISODate);
		if (item.msgId) {
			_items.push_back(std::move(item));
		}
	}
}

void BookmarksManager::save() {
	QJsonArray arr;
	for (const auto &item : _items) {
		QJsonObject obj;
		obj["peer"] = QString::number(item.msgId.peer.value);
		obj["msg"] = QString::number(item.msgId.msg.bare);
		obj["text"] = item.previewText;
		obj["note"] = item.note;
		obj["cat"] = item.category;
		obj["date"] = item.savedAt.toString(Qt::ISODate);
		arr.append(obj);
	}
	const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Compact);
	Core::App().settings().writePref<QByteArray>(kBookmarksPref, bytes);
}

void BookmarksManager::addBookmark(FullMsgId id, const QString &text, const QString &note, const QString &category) {
	removeBookmark(id);
	BookmarkItem item;
	item.msgId = id;
	item.previewText = text.left(250);
	item.note = note;
	item.category = category.trimmed();
	item.savedAt = QDateTime::currentDateTime();
	_items.push_back(std::move(item));
	save();
}

void BookmarksManager::removeBookmark(FullMsgId id) {
	auto it = std::remove_if(_items.begin(), _items.end(), [&](const BookmarkItem &item) {
		return item.msgId == id;
	});
	if (it != _items.end()) {
		_items.erase(it, _items.end());
		save();
	}
}

bool BookmarksManager::isBookmarked(FullMsgId id) const {
	for (const auto &item : _items) {
		if (item.msgId == id) return true;
	}
	return false;
}

std::vector<BookmarkItem> BookmarksManager::list(const QString &categoryFilter) const {
	if (categoryFilter.isEmpty()) {
		return _items;
	}
	std::vector<BookmarkItem> result;
	for (const auto &item : _items) {
		if (item.category.compare(categoryFilter, Qt::CaseInsensitive) == 0) {
			result.push_back(item);
		}
	}
	return result;
}

std::vector<QString> BookmarksManager::categories() const {
	std::vector<QString> result;
	for (const auto &item : _items) {
		if (!item.category.isEmpty() && std::find(result.begin(), result.end(), item.category) == result.end()) {
			result.push_back(item.category);
		}
	}
	return result;
}

} // namespace Rekogram
