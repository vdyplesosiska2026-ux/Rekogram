/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_rekogram.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

#include "boxes/background_box.h"
#include "core/application.h"
#include "core/version.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "rekogram/rekogram_ai_chat.h"
#include "rekogram/rekogram_backup.h"
#include "rekogram/rekogram_badge.h"
#include "rekogram/rekogram_bookmarks.h"
#include "rekogram/rekogram_command_palette.h"
#include "rekogram/rekogram_plugin_system.h"
#include "rekogram/rekogram_quick_replies.h"
#include "rekogram/rekogram_reminders.h"
#include "rekogram/rekogram_settings.h"
#include "rekogram/rekogram_streamer_mode.h"
#include "settings/sections/settings_main.h"
#include "settings/sections/settings_shortcuts.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "ui/basic_click_handlers.h"
#include "ui/boxes/confirm_box.h"
#include "ui/painter.h"
#include "ui/toast/toast.h"
#include "ui/vertical_list.h"
#include "ui/widgets/box_content_divider.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/themes/window_theme.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

using namespace Builder;

class RekogramSection final : public Section<RekogramSection> {
public:
	RekogramSection(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();
};

RekogramSection::RekogramSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

rpl::producer<QString> RekogramSection::title() {
	return rpl::single(QString::fromUtf8("Настройки Rekogram"));
}

void SetupAppearance(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Внешний вид и темы")));

	// Dark / Light theme toggle
	builder.addButton({
		.id = u"rekogram/night_mode"_q,
		.title = rpl::single(QString::fromUtf8("Переключить тёмную/светлую тему")),
		.icon = { &st::menuIconNightMode },
		.onClick = [=] {
			Window::Theme::ToggleNightMode();
		},
	});

	// Chat background chooser
	builder.addButton({
		.id = u"rekogram/chat_bg"_q,
		.title = rpl::single(QString::fromUtf8("Фон для чатов")),
		.icon = { &st::menuIconShowInFolder },
		.onClick = [=] {
			controller->show(Box<BackgroundBox>(controller));
		},
	});

	// Reset appearance to default
	builder.addButton({
		.id = u"rekogram/reset_appearance"_q,
		.title = rpl::single(QString::fromUtf8("Сбросить оформление к стандартному")),
		.icon = { &st::menuIconRestore },
		.onClick = [=] {
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8("Сбросить оформление Rekogram до исходных параметров?"),
				.confirmed = [=](Fn<void()> &&close) {
					Rekogram::SettingsManager::Instance().resetToDefaults();
					close();
					Ui::Toast::Show(QString::fromUtf8("Настройки внешнего вида сброшены"));
				},
				.confirmText = QString::fromUtf8("Сбросить"),
			}));
		},
	});

	builder.addSkip();
}

void SetupChatList(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Список чатов (Chat List)")));

	// Compact chat list toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Компактный вид списка диалогов"),
				Rekogram::SettingsManager::Instance().compactChatList(),
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::SettingsManager::Instance().setCompactChatList(checked);
			Ui::Toast::Show(checked
				? QString::fromUtf8("Включен компактный список диалогов")
				: QString::fromUtf8("Включен стандартный список диалогов"));
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	builder.addSkip();
}

void SetupInterfaceLab(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Лаборатория интерфейса (Interface Lab)")));

	// Round avatars toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Использовать круглые аватары"),
				Rekogram::SettingsManager::Instance().avatarRadius() > 0,
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::SettingsManager::Instance().setAvatarRadius(checked ? 50 : 0);
			Ui::Toast::Show(checked
				? QString::fromUtf8("Установлены круглые аватары")
				: QString::fromUtf8("Установлены квадратные аватары"));
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	// Creator Crest Badge preview toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Отображать знак создателя (Creator Badge)"),
				Rekogram::SettingsManager::Instance().showCreatorBadge(),
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::SettingsManager::Instance().setShowCreatorBadge(checked);
			Ui::Toast::Show(checked
				? QString::fromUtf8("Creator Badge включен")
				: QString::fromUtf8("Creator Badge скрыт"));
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	builder.addSkip();
}

void SetupToolsSection(SectionBuilder &builder) {
	const auto controller = builder.controller();
	const auto showOther = builder.showOther();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Инструменты и быстрый доступ")));

	builder.addButton({
		.id = u"rekogram/shortcuts"_q,
		.title = rpl::single(QString::fromUtf8("Настройка горячих клавиш")),
		.icon = { &st::menuIconShortcut },
		.onClick = [=] { showOther(ShortcutsId()); },
	});

	builder.addButton({
		.id = u"rekogram/bookmarks_count"_q,
		.title = rpl::single(QString::fromUtf8("Сохранённые закладки сообщений")),
		.icon = { &st::menuIconShowInFolder },
		.onClick = [=] {
			const auto count = Rekogram::BookmarksManager::Instance().list().size();
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8("Всего сохранено сообщений в закладках: %1.\n\nДобавлять сообщения в закладки можно через контекстное меню любого сообщения в чате.").arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
				.title = QString::fromUtf8("Закладки Rekogram"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/reminders_count"_q,
		.title = rpl::single(QString::fromUtf8("Активные напоминания")),
		.icon = { &st::menuIconTimer },
		.onClick = [=] {
			const auto count = Rekogram::RemindersManager::Instance().activeReminders().size();
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8("Количество активных напоминаний о сообщениях: %1.\n\nУстановить напоминание можно через контекстное меню любого сообщения в чате.").arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
				.title = QString::fromUtf8("Напоминания Rekogram"),
			}));
		},
	});

	builder.addSkip();
}

void SetupBackupSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Резервное копирование (Settings Backup)")));

	builder.addButton({
		.id = u"rekogram/export_settings"_q,
		.title = rpl::single(QString::fromUtf8("Экспорт настроек Rekogram в JSON")),
		.icon = { &st::menuIconExportTheme },
		.onClick = [=] {
			const auto json = Rekogram::BackupManager::ExportSettings();
			QGuiApplication::clipboard()->setText(QString::fromUtf8(json));
			Ui::Toast::Show(QString::fromUtf8("Конфигурация JSON скопирована в буфер обмена"));
		},
	});

	builder.addButton({
		.id = u"rekogram/import_settings"_q,
		.title = rpl::single(QString::fromUtf8("Импорт настроек из буфера обмена")),
		.icon = { &st::menuIconImportTheme },
		.onClick = [=] {
			const auto clipboardText = QGuiApplication::clipboard()->text().toUtf8();
			QString error;
			if (Rekogram::BackupManager::ImportSettings(clipboardText, &error)) {
				Ui::Toast::Show(QString::fromUtf8("Настройки Rekogram успешно импортированы!"));
			} else {
				controller->show(Ui::MakeInformBox(error));
			}
		},
	});

	builder.addSkip();
}

void SetupAIChatSection(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Rekogram AI Chat")));

	builder.addButton({
		.id = u"rekogram/ai_status"_q,
		.title = rpl::single(QString::fromUtf8("Статус AI Chat: Архитектура подготовлена")),
		.icon = { &st::menuIconDiscussion },
		.onClick = [=] {
			const auto count = Rekogram::AIChatManager::Instance().messages().size();
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8(
					"Провайдер: %1\n"
					"Сообщений в локальной истории: %2\n\n"
					"Модуль диалога полностью сохраняет оригинальный стиль Telegram Desktop. Подключение внешних нейросетевых API будет осуществляться на следующих этапах без изменения интерфейса переписки.")
					.arg(Rekogram::AIChatManager::Instance().providerName())
					.arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
				.title = QString::fromUtf8("Rekogram AI Chat"),
			}));
		},
	});

	builder.addSkip();
}

void SetupPrivacyAndSecurity(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Приватность и защита (Privacy & Security)")));

	// Streamer Mode Toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Режим стримера (Streamer Shield)"),
				Rekogram::StreamerModeManager::Instance().enabled(),
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::StreamerModeManager::Instance().setEnabled(checked);
			Ui::Toast::Show(checked
				? QString::fromUtf8("Режим стримера включен: данные маскируются")
				: QString::fromUtf8("Режим стримера выключен"));
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	builder.addSkip();
}

void SetupPluginsAndCommands(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Экосистема Rekogram (Плагины и команды)")));

	builder.addButton({
		.id = u"rekogram/palette_preview"_q,
		.title = rpl::single(QString::fromUtf8("Палитра быстрых команд (Command Palette)")),
		.icon = { &st::menuIconShortcut },
		.onClick = [=] {
			const auto cmds = Rekogram::CommandPaletteManager::Instance().commands();
			QString text = QString::fromUtf8("Зарегистрированные системные команды Rekogram:\n\n");
			for (const auto &c : cmds) {
				text += QString("[%1] %2 (%3)\n").arg(c.category, c.title, c.shortcut.isEmpty() ? "действие" : c.shortcut);
			}
			controller->show(Ui::MakeConfirmBox({
				.text = text,
				.confirmText = QString::fromUtf8("Понятно"),
				.title = QString::fromUtf8("Командная палитра Rekogram"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/quick_replies_info"_q,
		.title = rpl::single(QString::fromUtf8("Шаблоны быстрых ответов (Quick Replies)")),
		.icon = { &st::menuIconDiscussion },
		.onClick = [=] {
			const auto list = Rekogram::QuickRepliesManager::Instance().list();
			QString text = QString::fromUtf8("Настроенные быстрые сниппеты ответов:\n\n");
			for (const auto &item : list) {
				text += QString("%1 -> \"%2\"\n").arg(item.keyword, item.text);
			}
			controller->show(Ui::MakeConfirmBox({
				.text = text,
				.confirmText = QString::fromUtf8("Закрыть"),
				.title = QString::fromUtf8("Быстрые ответы Rekogram"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/plugins_status"_q,
		.title = rpl::single(QString::fromUtf8("Менеджер плагинов (Rekogram Extensions)")),
		.icon = { &st::menuIconShowInFolder },
		.onClick = [=] {
			const auto plugins = Rekogram::PluginManager::Instance().plugins();
			QString text = QString::fromUtf8("Активные встроенные расширения:\n\n");
			for (const auto &p : plugins) {
				text += QString("• %1 v%2 — %3 [%4]\n").arg(p.name, p.version, p.description, p.enabled ? "Вкл" : "Выкл");
			}
			controller->show(Ui::MakeConfirmBox({
				.text = text,
				.confirmText = QString::fromUtf8("Готово"),
				.title = QString::fromUtf8("Модули расширения Rekogram"),
			}));
		},
	});

	builder.addSkip();
}

void SetupRekogramHub(SectionBuilder &builder) {
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Rekogram Hub — О приложении")));

	builder.addButton({
		.id = u"rekogram/hub_info"_q,
		.title = rpl::single(QString::fromUtf8("Версия: Rekogram v") + AppVersionStr),
		.icon = { &st::menuIconEmojiObjects },
		.onClick = [=] {
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8(
					"Rekogram — самостоятельный клиент на базе исходного кода Telegram Desktop.\n\n"
					"Версия: %1\n"
					"Исходный репозиторий: Telegram Desktop\n"
					"Лицензия: GNU GPL v3 с исключениями OpenSSL\n\n"
					"Проект сохраняет полную совместимость с Telegram API и безопасность данных.")
					.arg(AppVersionStr),
				.confirmText = QString::fromUtf8("Понятно"),
				.title = QString::fromUtf8("Rekogram Desktop"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/github"_q,
		.title = rpl::single(QString::fromUtf8("Репозиторий на GitHub")),
		.icon = { &st::menuIconLinks },
		.onClick = [=] {
			UrlClickHandler::Open(u"https://github.com/vdyplesosiska2026-ux/Rekogram"_q);
		},
	});

	builder.addSkip();
}

void RekogramSection::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	const SectionBuildMethod buildMethod = [=](
			not_null<Ui::VerticalLayout*> container,
			not_null<Window::SessionController*> controller,
			Fn<void(Type)> showOther,
			rpl::producer<> showFinished) {
		auto &lifetime = container->lifetime();
		const auto highlights = lifetime.make_state<HighlightRegistry>();
		const auto isPaused = Window::PausedIn(
			controller,
			Window::GifPauseReason::Layer);

		auto builder = SectionBuilder(WidgetContext{
			.container = container,
			.controller = controller,
			.showOther = std::move(showOther),
			.isPaused = isPaused,
			.highlights = highlights,
		});

		SetupAppearance(builder);
		builder.addDivider();
		builder.addSkip();

		SetupChatList(builder);
		builder.addDivider();
		builder.addSkip();

		SetupInterfaceLab(builder);
		builder.addDivider();
		builder.addSkip();

		SetupToolsSection(builder);
		builder.addDivider();
		builder.addSkip();

		SetupBackupSection(builder);
		builder.addDivider();
		builder.addSkip();

		SetupAIChatSection(builder);
		builder.addDivider();
		builder.addSkip();

		SetupPrivacyAndSecurity(builder);
		builder.addDivider();
		builder.addSkip();

		SetupPluginsAndCommands(builder);
		builder.addDivider();
		builder.addSkip();

		SetupRekogramHub(builder);
	};

	build(content, buildMethod);
	Ui::ResizeFitChild(this, content);
}

const auto kMeta = BuildHelper({
	.id = RekogramSection::Id(),
	.parentId = MainId(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconSettings,
}, [](SectionBuilder &builder) {
	builder.add(nullptr, [] {
		return SearchEntry{
			.id = u"rekogram/settings"_q,
			.title = QString::fromUtf8("Настройки Rekogram"),
			.keywords = { u"rekogram"_q, u"custom"_q, u"appearance"_q, u"themes"_q },
		};
	});
});

} // namespace

Type RekogramId() {
	return RekogramSection::Id();
}

} // namespace Settings
