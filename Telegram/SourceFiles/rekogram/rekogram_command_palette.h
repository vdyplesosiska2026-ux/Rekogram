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

struct CommandItem {
	QString id;
	QString title;
	QString category;
	QString shortcut;
};

class CommandPaletteManager final {
public:
	static CommandPaletteManager &Instance();

	[[nodiscard]] std::vector<CommandItem> commands() const;
	[[nodiscard]] std::vector<CommandItem> search(const QString &query) const;

private:
	CommandPaletteManager();
	std::vector<CommandItem> _commands;
};

} // namespace Rekogram
