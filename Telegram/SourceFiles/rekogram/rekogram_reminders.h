/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "data/data_msg_id.h"
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <vector>

namespace Rekogram {

struct ReminderItem {
	FullMsgId msgId;
	QString text;
	QDateTime triggerTime;
	bool completed = false;
};

class RemindersManager final {
public:
	static RemindersManager &Instance();

	void addReminder(FullMsgId id, const QString &text, const QDateTime &time);
	void removeReminder(FullMsgId id);
	void markCompleted(FullMsgId id);
	void reload() { load(); }

	[[nodiscard]] std::vector<ReminderItem> activeReminders() const;

private:
	RemindersManager();
	void load();
	void save();
	void checkDueReminders();

	std::vector<ReminderItem> _items;
	QTimer _checkTimer;
};

} // namespace Rekogram
