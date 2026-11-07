/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_command_palette.h"

namespace Rekogram {

CommandPaletteManager &CommandPaletteManager::Instance() {
	static CommandPaletteManager instance;
	return instance;
}

CommandPaletteManager::CommandPaletteManager() {
	_commands = {
		{ "theme.toggle", "Переключить ночную/светлую тему", "Внешний вид", "Ctrl+Shift+N" },
		{ "settings.rekogram", "Открыть настройки Rekogram", "Навигация", "Ctrl+," },
		{ "streamer.toggle", "Включить/выключить режим стримера", "Приватность", "Ctrl+Shift+S" },
		{ "backup.export", "Экспорт конфигурации настроек в JSON", "Инструменты", "" },
		{ "ai.status", "Проверить статус Rekogram AI", "AI & Интеллект", "" },
		{ "bookmarks.view", "Просмотр закладок сообщений", "Сообщения", "" },
		{ "shortcuts.open", "Настройка горячих клавиш", "Управление", "" }
	};
}

std::vector<CommandItem> CommandPaletteManager::commands() const {
	return _commands;
}

std::vector<CommandItem> CommandPaletteManager::search(const QString &query) const {
	if (query.trimmed().isEmpty()) return _commands;

	std::vector<CommandItem> result;
	for (const auto &cmd : _commands) {
		if (cmd.title.contains(query, Qt::CaseInsensitive)
			|| cmd.category.contains(query, Qt::CaseInsensitive)
			|| cmd.id.contains(query, Qt::CaseInsensitive)) {
			result.push_back(cmd);
		}
	}
	return result;
}

} // namespace Rekogram
