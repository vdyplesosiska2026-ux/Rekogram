/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_command_palette.h"

#include "rekogram/rekogram_ai_chat.h"
#include "rekogram/rekogram_ai_chat_ui.h"
#include "rekogram/rekogram_backup.h"
#include "rekogram/rekogram_bookmarks.h"
#include "rekogram/rekogram_bookmarks_ui.h"
#include "rekogram/rekogram_premium.h"
#include "rekogram/rekogram_settings.h"
#include "rekogram/rekogram_streamer_mode.h"
#include "window/window_session_controller.h"
#include "window/themes/window_theme.h"
#include "ui/boxes/confirm_box.h"
#include "ui/toast/toast.h"
#include "settings/sections/settings_rekogram.h"
#include "settings/sections/settings_shortcuts.h"
#include "core/application.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtCore/QFile>
#include <QtGui/QKeyEvent>

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
		{ "ai.open", "Открыть чат с Rekogram AI", "AI & Интеллект", "" },
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

namespace CommandPalette {
namespace {

class PaletteDialog final : public QDialog {
public:
	PaletteDialog(
			not_null<Window::SessionController*> controller,
			const std::vector<CommandItem> &commands)
	: QDialog(controller->widget())
	, _commands(commands) {
		setWindowTitle(QString::fromUtf8("Палитра команд Rekogram"));
		setModal(true);
		setMinimumWidth(480);
		setMaximumWidth(640);

		auto *layout = new QVBoxLayout(this);
		layout->setContentsMargins(12, 12, 12, 12);
		layout->setSpacing(8);

		_input = new QLineEdit(this);
		_input->setPlaceholderText(QString::fromUtf8("Введите команду..."));
		_input->setClearButtonEnabled(true);
		layout->addWidget(_input);

		_list = new QListWidget(this);
		_list->setSelectionMode(QAbstractItemView::SingleSelection);
		layout->addWidget(_list, 1);

		auto *buttons = new QHBoxLayout;
		buttons->addStretch(1);
		auto *closeBtn = new QPushButton(QString::fromUtf8("Закрыть"), this);
		connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
		buttons->addWidget(closeBtn);
		layout->addLayout(buttons);

		connect(_input, &QLineEdit::textChanged, this, &PaletteDialog::filterCommands);
		connect(_list, &QListWidget::itemActivated, this, &PaletteDialog::activateItem);
		connect(_list, &QListWidget::itemClicked, this, &PaletteDialog::activateItem);

		filterCommands(QString());
		_input->setFocus();
	}

protected:
	void keyPressEvent(QKeyEvent *event) override {
		if (event->key() == Qt::Key_Escape) {
			reject();
			return;
		}
		if (event->key() == Qt::Key_Down) {
			const auto row = _list->currentRow() + 1;
			if (row < _list->count()) _list->setCurrentRow(row);
			return;
		}
		if (event->key() == Qt::Key_Up) {
			const auto row = _list->currentRow() - 1;
			if (row >= 0) _list->setCurrentRow(row);
			return;
		}
		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
			activateCurrent();
			return;
		}
		QDialog::keyPressEvent(event);
	}

private:
	void filterCommands(const QString &query) {
		_list->clear();
		const auto filtered = CommandPaletteManager::Instance().search(query);
		for (const auto &cmd : filtered) {
			auto *item = new QListWidgetItem(_list);
			item->setText(cmd.title);
			item->setData(Qt::UserRole, cmd.id);
			if (!cmd.category.isEmpty()) {
				item->setToolTip(cmd.category + (cmd.shortcut.isEmpty() ? QString() : " — " + cmd.shortcut));
			}
		}
		if (_list->count() > 0) {
			_list->setCurrentRow(0);
		}
	}

	void activateItem(QListWidgetItem *item) {
		if (!item) return;
		const auto id = item->data(Qt::UserRole).toString();
		for (const auto &cmd : _commands) {
			if (cmd.id == id && cmd.action) {
				accept();
				cmd.action();
				return;
			}
		}
	}

	void activateCurrent() {
		activateItem(_list->currentItem());
	}

	const std::vector<CommandItem> &_commands;
	QLineEdit *_input = nullptr;
	QListWidget *_list = nullptr;
};

void ExecuteCommand(const QString &id, not_null<Window::SessionController*> controller) {
	if (id == "theme.toggle") {
		Window::Theme::ToggleNightMode();
	} else if (id == "settings.rekogram") {
		controller->showSettings(Settings::RekogramId());
	} else if (id == "streamer.toggle") {
		auto &sm = StreamerModeManager::Instance();
		sm.setEnabled(!sm.enabled());
		Ui::Toast::Show(sm.enabled()
			? QString::fromUtf8("Режим стримера включён")
			: QString::fromUtf8("Режим стримера выключен"));
	} else if (id == "backup.export") {
		const auto data = BackupManager::ExportSettings();
		const auto path = QFileDialog::getSaveFileName(
			nullptr,
			QString::fromUtf8("Экспорт конфигурации Rekogram"),
			QString::fromUtf8("rekogram_backup.json"),
			QString::fromUtf8("JSON (*.json)"));
		if (!path.isEmpty()) {
			QFile file(path);
			if (file.open(QIODevice::WriteOnly)) {
				file.write(data);
				Ui::Toast::Show(QString::fromUtf8("Конфигурация экспортирована"));
			}
		}
	} else if (id == "ai.status") {
		const auto &ai = AIChatManager::Instance();
		Ui::Toast::Show(ai.isConfigured()
			? QString::fromUtf8("Rekogram AI: настроен (%1)").arg(ai.providerName())
			: QString::fromUtf8("Rekogram AI: не настроен"));
	} else if (id == "ai.open") {
		AIChatUI::Show(controller);
	} else if (id == "bookmarks.view") {
		BookmarksUI::Show(controller);
	} else if (id == "shortcuts.open") {
		controller->showSettings(Settings::ShortcutsId());
	}
}

} // namespace

void Show(not_null<Window::SessionController*> controller) {
	auto &manager = CommandPaletteManager::Instance();
	auto commands = manager.commands();
	for (auto &cmd : commands) {
		cmd.action = [controller, id = cmd.id]() {
			ExecuteCommand(id, controller);
		};
	}

	PaletteDialog dialog(controller, commands);
	dialog.exec();
}

} // namespace CommandPalette

} // namespace Rekogram
