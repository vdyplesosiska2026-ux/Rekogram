/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_reminders.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kRemindersPref = "reko_reminders_data";

} // namespace

RemindersManager &RemindersManager::Instance() {
	static RemindersManager instance;
	return instance;
}

RemindersManager::RemindersManager() {
	load();
}

void RemindersManager::load() {
	_items.clear();
	const auto raw = Core::App().settings().readPref<QByteArray>(kRemindersPref, QByteArray());
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
		ReminderItem item;
		item.msgId.peer = PeerId(obj["peer"].toString().toULongLong());
		item.msgId.msg = MsgId(obj["msg"].toString().toLongLong());
		item.text = obj["text"].toString();
		item.triggerTime = QDateTime::fromString(obj["trigger"].toString(), Qt::ISODate);
		item.completed = obj["done"].toBool(false);
		if (item.msgId) {
			_items.push_back(std::move(item));
		}
	}
}

void RemindersManager::save() {
	QJsonArray arr;
	for (const auto &item : _items) {
		QJsonObject obj;
		obj["peer"] = QString::number(item.msgId.peer.value);
		obj["msg"] = QString::number(item.msgId.msg.bare);
		obj["text"] = item.text;
		obj["trigger"] = item.triggerTime.toString(Qt::ISODate);
		obj["done"] = item.completed;
		arr.append(obj);
	}
	const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Compact);
	Core::App().settings().writePref<QByteArray>(kRemindersPref, bytes);
}

void RemindersManager::addReminder(FullMsgId id, const QString &text, const QDateTime &time) {
	removeReminder(id);
	ReminderItem item;
	item.msgId = id;
	item.text = text.left(200);
	item.triggerTime = time;
	item.completed = false;
	_items.push_back(std::move(item));
	save();
}

void RemindersManager::removeReminder(FullMsgId id) {
	auto it = std::remove_if(_items.begin(), _items.end(), [&](const ReminderItem &item) {
		return item.msgId == id;
	});
	if (it != _items.end()) {
		_items.erase(it, _items.end());
		save();
	}
}

void RemindersManager::markCompleted(FullMsgId id) {
	for (auto &item : _items) {
		if (item.msgId == id) {
			item.completed = true;
			save();
			break;
		}
	}
}

std::vector<ReminderItem> RemindersManager::activeReminders() const {
	std::vector<ReminderItem> result;
	for (const auto &item : _items) {
		if (!item.completed) {
			result.push_back(item);
		}
	}
	return result;
}

} // namespace Rekogram
