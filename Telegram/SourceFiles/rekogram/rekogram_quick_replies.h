/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <vector>

namespace Rekogram {

struct QuickReplyItem {
	QString keyword;
	QString text;
};

class QuickRepliesManager final {
public:
	static QuickRepliesManager &Instance();

	void addTemplate(const QString &keyword, const QString &text);
	void removeTemplate(const QString &keyword);
	[[nodiscard]] std::vector<QuickReplyItem> list() const;
	[[nodiscard]] QString findText(const QString &keyword) const;

private:
	QuickRepliesManager();
	void load();
	void save();

	std::vector<QuickReplyItem> _items;
};

} // namespace Rekogram
