/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_rekogram.h"

#include "boxes/background_box.h"
#include "core/application.h"
#include "core/version.h"
#include "main/main_session.h"
#include "rekogram/rekogram_ai_chat.h"
#include "rekogram/rekogram_backup.h"
#include "rekogram/rekogram_badge.h"
#include "rekogram/rekogram_bookmarks.h"
#include "rekogram/rekogram_reminders.h"
#include "rekogram/rekogram_settings.h"
#include "settings/sections/settings_main.h"
#include "settings/sections/settings_shortcuts.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
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
			Window::Theme::ToggleNightModeWithConfirmation(
				&controller->window());
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
		.icon = { &st::menuIconReset },
		.onClick = [=] {
			controller->show(Ui::MakeConfirmBox({
				.text = QString::fromUtf8("Сбросить оформление Rekogram до исходных параметров?"),
				.confirmed = [=](Fn<void()> &&close) {
					Rekogram::SettingsManager::Instance().resetToDefaults();
					close();
				},
				.confirmText = QString::fromUtf8("Сбросить"),
			}));
		},
	});

	builder.addSkip();
}

void SetupChatList(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Список чатов (Advanced Chat List)")));

	// Compact chat list toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Компактный режим списка чатов"),
				Rekogram::SettingsManager::Instance().compactChatList(),
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::SettingsManager::Instance().setCompactChatList(checked);
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	builder.addSkip();
}

void SetupInterfaceLab(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Лаборатория интерфейса (Interface Lab)")));

	// Developer / Experimental mode toggle
	builder.add([=](const WidgetContext &ctx) {
		auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
		const auto checkbox = wrap->add(
			object_ptr<Ui::Checkbox>(
				wrap.data(),
				QString::fromUtf8("Включить экспериментальные возможности"),
				Rekogram::SettingsManager::Instance().devMode(),
				st::defaultCheckbox),
			st::settingsCheckboxPadding);

		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Rekogram::SettingsManager::Instance().setDevMode(checked);
		}, checkbox->lifetime());

		return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
	});

	builder.addSkip();
}

void SetupShortcutsAndTools(SectionBuilder &builder) {
	const auto showOther = builder.showOther();
	const auto controller = builder.controller();

	builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Горячие клавиши и управление (Rekogram Shortcuts)")));

	builder.addButton({
		.id = u"rekogram/shortcuts"_q,
		.title = rpl::single(QString::fromUtf8("Настройка горячих клавиш")),
		.icon = { &st::menuIconKeyboard },
		.onClick = [=] { showOther(ShortcutsId()); },
	});

	builder.addButton({
		.id = u"rekogram/bookmarks_count"_q,
		.title = rpl::single(QString::fromUtf8("Сохранённые закладки сообщений")),
		.icon = { &st::menuIconShowInFolder },
		.onClick = [=] {
			const auto count = Rekogram::BookmarksManager::Instance().list().size();
			controller->show(Ui::MakeConfirmBox({
				.title = QString::fromUtf8("Закладки Rekogram"),
				.text = QString::fromUtf8("Всего сохранено сообщений в закладках: %1.\n\nДобавлять сообщения в закладки можно через контекстное меню любого сообщения в чате.").arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/reminders_count"_q,
		.title = rpl::single(QString::fromUtf8("Активные напоминания")),
		.icon = { &st::menuIconHistory },
		.onClick = [=] {
			const auto count = Rekogram::RemindersManager::Instance().activeReminders().size();
			controller->show(Ui::MakeConfirmBox({
				.title = QString::fromUtf8("Напоминания Rekogram"),
				.text = QString::fromUtf8("Количество активных напоминаний о сообщениях: %1.\n\nУстановить напоминание можно через контекстное меню любого сообщения в чате.").arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
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
		.icon = { &st::menuIconSave },
		.onClick = [=] {
			const auto json = Rekogram::BackupManager::ExportSettings();
			QGuiApplication::clipboard()->setText(QString::fromUtf8(json));
			Ui::Toast::Show(QString::fromUtf8("Конфигурация JSON скопирована в буфер обмена"));
		},
	});

	builder.addButton({
		.id = u"rekogram/import_settings"_q,
		.title = rpl::single(QString::fromUtf8("Импорт настроек из буфера обмена")),
		.icon = { &st::menuIconCloudDownload },
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
				.title = QString::fromUtf8("Rekogram AI Chat"),
				.text = QString::fromUtf8(
					"Провайдер: %1\n"
					"Сообщений в локальной истории: %2\n\n"
					"Модуль диалога полностью сохраняет оригинальный стиль Telegram Desktop. Подключение внешних нейросетевых API будет осуществляться на следующих этапах без изменения интерфейса переписки.")
					.arg(Rekogram::AIChatManager::Instance().providerName())
					.arg(count),
				.confirmText = QString::fromUtf8("Понятно"),
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
				.title = QString::fromUtf8("Rekogram Desktop"),
				.text = QString::fromUtf8(
					"Rekogram — самостоятельный клиент на базе исходного кода Telegram Desktop.\n\n"
					"Версия: %1\n"
					"Исходный репозиторий: Telegram Desktop\n"
					"Лицензия: GNU GPL v3 с исключениями OpenSSL\n\n"
					"Проект сохраняет полную совместимость с Telegram API и безопасность данных.")
					.arg(AppVersionStr),
				.confirmText = QString::fromUtf8("Понятно"),
			}));
		},
	});

	builder.addButton({
		.id = u"rekogram/github"_q,
		.title = rpl::single(QString::fromUtf8("Репозиторий на GitHub")),
		.icon = { &st::menuIconLinks },
		.onClick = [=] {
			File::OpenUrl(u"https://github.com/rekogram"_q);
		},
	});

	builder.addSkip();
}

void RekogramSection::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	const auto window = controller();

	const SectionBuildMethod buildMethod = [](
			not_null<Ui::VerticalLayout*> container,
			not_null<Window::SessionController*> controller,
			Fn<void(Type)> showOther,
			rpl::producer<> showFinished) {
		auto builder = SectionBuilder(WidgetContext{
			.container = container,
			.controller = controller,
			.showOther = std::move(showOther),
		});

		builder.addDivider();
		builder.addSkip();

		SetupAppearance(builder);
		builder.addDivider();
		builder.addSkip();

		SetupChatList(builder);
		builder.addDivider();
		builder.addSkip();

		SetupInterfaceLab(builder);
		builder.addDivider();
		builder.addSkip();

		SetupShortcutsAndTools(builder);
		builder.addDivider();
		builder.addSkip();

		SetupBackupSection(builder);
		builder.addDivider();
		builder.addSkip();

		SetupAIChatSection(builder);
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
	.title = [] { return tr::lng_menu_settings(); },
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

namespace Builder {

SectionBuildMethod RekogramSectionBuild = kMeta.build;

} // namespace Builder
} // namespace Settings
