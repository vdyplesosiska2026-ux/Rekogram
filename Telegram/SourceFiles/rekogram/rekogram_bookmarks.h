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
#include <vector>

namespace Rekogram {

struct BookmarkItem {
	FullMsgId msgId;
	QString note;
	QString category;
	QString previewText;
	QDateTime savedAt;
};

class BookmarksManager final {
public:
	static BookmarksManager &Instance();

	void addBookmark(FullMsgId id, const QString &text, const QString &note = QString(), const QString &category = QString());
	void removeBookmark(FullMsgId id);
	[[nodiscard]] bool isBookmarked(FullMsgId id) const;

	[[nodiscard]] std::vector<BookmarkItem> list(const QString &categoryFilter = QString()) const;
	[[nodiscard]] std::vector<QString> categories() const;

private:
	BookmarksManager();
	void load();
	void save();

	std::vector<BookmarkItem> _items;
};

} // namespace Rekogram
