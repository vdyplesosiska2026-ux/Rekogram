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
#include "rekogram/rekogram_inspector.h"
#include "rekogram/rekogram_plugin_system.h"
#include "rekogram/rekogram_premium.h"
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

// Forward declarations of Section classes
class RekogramSection;
class RekogramGeneralSection;
class RekogramAppearanceSection;
class RekogramPrivacySection;
class RekogramChatsSection;
class RekogramMessagesSection;
class RekogramAISection;
class RekogramPluginsSection;
class RekogramAdvancedSection;
class RekogramPremiumSection;
class RekogramAboutSection;

// 1. General Section
class RekogramGeneralSection final : public Section<RekogramGeneralSection> {
public:
	RekogramGeneralSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Общие настройки"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Резервное копирование")));
			builder.addButton({
				.id = u"reko_gen/export"_q,
				.title = rpl::single(QString::fromUtf8("Экспорт конфигурации настроек (JSON)")),
				.icon = { &st::menuIconDownload },
				.onClick = [=] {
					Rekogram::BackupManager::Instance().exportToJson();
					Ui::Toast::Show(QString::fromUtf8("Резервная копия настроек скопирована в буфер"));
				},
			});
			builder.addButton({
				.id = u"reko_gen/import"_q,
				.title = rpl::single(QString::fromUtf8("Импорт конфигурации настроек из буфера")),
				.icon = { &st::menuIconRestore },
				.onClick = [=] {
					const auto text = QGuiApplication::clipboard()->text();
					if (Rekogram::BackupManager::Instance().importFromJson(text)) {
						Ui::Toast::Show(QString::fromUtf8("Настройки успешно восстановлены"));
					} else {
						Ui::Toast::Show(QString::fromUtf8("Ошибка: в буфере нет корректного JSON Rekogram"));
					}
				},
			});

			builder.addSkip();
			builder.addDivider();
			builder.addSkip();

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Сброс параметров")));
			builder.addButton({
				.id = u"reko_gen/reset"_q,
				.title = rpl::single(QString::fromUtf8("Сбросить все настройки Rekogram к стандартным")),
				.icon = { &st::menuIconDelete },
				.onClick = [=] {
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8("Сбросить все настройки Rekogram до заводских параметров?"),
						.confirmed = [=](Fn<void()> &&close) {
							Rekogram::SettingsManager::Instance().resetToDefaults();
							close();
							Ui::Toast::Show(QString::fromUtf8("Все настройки Rekogram сброшены"));
						},
						.confirmText = QString::fromUtf8("Сбросить"),
					}));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 2. Appearance Section
class RekogramAppearanceSection final : public Section<RekogramAppearanceSection> {
public:
	RekogramAppearanceSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Внешний вид и темы"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Темы и фон")));
			builder.addButton({
				.id = u"reko_app/night_mode"_q,
				.title = rpl::single(QString::fromUtf8("Переключить ночной/дневной режим")),
				.icon = { &st::menuIconNightMode },
				.onClick = [=] { Window::Theme::ToggleNightMode(); },
			});
			builder.addButton({
				.id = u"reko_app/chat_bg"_q,
				.title = rpl::single(QString::fromUtf8("Выбрать фон для чатов")),
				.icon = { &st::menuIconShowInFolder },
				.onClick = [=] { controller->show(Box<BackgroundBox>(controller)); },
			});

			builder.addSkip();
			builder.addDivider();
			builder.addSkip();

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Список чатов и аватары")));
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

			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Использовать круглые аватары (выкл — скругленные квадраты)"),
						Rekogram::SettingsManager::Instance().avatarRadius() > 0,
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::SettingsManager::Instance().setAvatarRadius(checked ? 100 : 0);
					Ui::Toast::Show(checked
						? QString::fromUtf8("Установлены круглые аватары")
						: QString::fromUtf8("Установлены скругленные квадратные аватары"));
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 3. Privacy Section
class RekogramPrivacySection final : public Section<RekogramPrivacySection> {
public:
	RekogramPrivacySection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Приватность и безопасность"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Streamer Shield — Режим защиты стрима")));
			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Включить Streamer Shield"),
						Rekogram::StreamerModeManager::Instance().enabled(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::StreamerModeManager::Instance().setEnabled(checked);
					Ui::Toast::Show(checked
						? QString::fromUtf8("Streamer Shield активирован")
						: QString::fromUtf8("Streamer Shield отключен"));
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Маскировать номер телефона в профиле"),
						Rekogram::StreamerModeManager::Instance().hidePhone(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::StreamerModeManager::Instance().setHidePhone(checked);
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Скрывать имя пользователя и юзернейм"),
						Rekogram::StreamerModeManager::Instance().hideUserData(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::StreamerModeManager::Instance().setHideUserData(checked);
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Скрывать текст сообщений в всплывающих уведомлениях"),
						Rekogram::StreamerModeManager::Instance().muteNotificationContent(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::StreamerModeManager::Instance().setMuteNotificationContent(checked);
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.addSkip();
			builder.addDivider();
			builder.addSkip();

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Privacy Dashboard")));
			builder.addButton({
				.id = u"reko_priv/dashboard"_q,
				.title = rpl::single(QString::fromUtf8("Сводка безопасности и локальной защиты")),
				.icon = { &st::menuIconLock },
				.onClick = [=] {
					const auto isShield = Rekogram::StreamerModeManager::Instance().enabled();
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Privacy Dashboard Rekogram:\n\n"
							"• Streamer Shield: %1\n"
							"• Маскировка номера: %2\n"
							"• Маскировка данных профиля: %3\n"
							"• Скрытие уведомлений: %4\n"
							"• Метаданные медиа: фильтруются перед отправкой\n"
							"• Протокол шифрования: MTProto 2.0 (End-to-End)")
							.arg(isShield ? QString::fromUtf8("Активен") : QString::fromUtf8("Отключен"))
							.arg(Rekogram::StreamerModeManager::Instance().hidePhone() ? QString::fromUtf8("Вкл") : QString::fromUtf8("Выкл"))
							.arg(Rekogram::StreamerModeManager::Instance().hideUserData() ? QString::fromUtf8("Вкл") : QString::fromUtf8("Выкл"))
							.arg(Rekogram::StreamerModeManager::Instance().muteNotificationContent() ? QString::fromUtf8("Вкл") : QString::fromUtf8("Выкл")),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Privacy Dashboard"),
					}));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 4. Chats Section
class RekogramChatsSection final : public Section<RekogramChatsSection> {
public:
	RekogramChatsSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Чаты и общение"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Быстрые ответы (Quick Replies)")));
			builder.addButton({
				.id = u"reko_chat/qr_info"_q,
				.title = rpl::single(QString::fromUtf8("Список активных шаблонов ответа (!hi, !ok, !thx)")),
				.icon = { &st::menuIconChatBubble },
				.onClick = [=] {
					const auto items = Rekogram::QuickRepliesManager::Instance().list();
					QString listStr;
					for (const auto &item : items) {
						listStr += item.keyword + u" -> "_q + item.text + u"\n"_q;
					}
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Быстрые ответы активируются вводом ключевого слова с восклицательным знаком:\n\n%1\n"
							"При отправке сообщение автоматически заменяется на полный текст.")
							.arg(listStr),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Шаблоны Quick Replies"),
					}));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 5. Messages Section
class RekogramMessagesSection final : public Section<RekogramMessagesSection> {
public:
	RekogramMessagesSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Сообщения и медиа"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Инспектор и закладки")));
			builder.addButton({
				.id = u"reko_msg/bookmarks"_q,
				.title = rpl::single(QString::fromUtf8("Закладки сообщений Rekogram")),
				.icon = { &st::menuIconShowInFolder },
				.onClick = [=] {
					const auto count = Rekogram::BookmarksManager::Instance().list().size();
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Сохранено сообщений в закладках: %1.\n\n"
							"Добавлять и удалять закладки можно через контекстное меню любого сообщения в чате.")
							.arg(count),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Закладки сообщений"),
					}));
				},
			});
			builder.addButton({
				.id = u"reko_msg/reminders"_q,
				.title = rpl::single(QString::fromUtf8("Напоминания о сообщениях")),
				.icon = { &st::menuIconTimer },
				.onClick = [=] {
					const auto count = Rekogram::RemindersManager::Instance().activeReminders().size();
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Активных напоминаний: %1.\n\n"
							"Установить напоминание можно через контекстное меню сообщения.")
							.arg(count),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Напоминания"),
					}));
				},
			});
			builder.addButton({
				.id = u"reko_msg/inspector_info"_q,
				.title = rpl::single(QString::fromUtf8("О функции Message Inspector")),
				.icon = { &st::menuIconEmojiObjects },
				.onClick = [=] {
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Message Inspector доступен при правом клике на сообщение в любом диалоге.\n\n"
							"Он отображает точный Message ID, Peer ID отправителя, метки времени Unix, типы вложений и технические параметры."),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Message Inspector"),
					}));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 6. AI Section
class RekogramAISection final : public Section<RekogramAISection> {
public:
	RekogramAISection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("AI и перевод"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Rekogram AI Ассистент")));
			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Включить встроенный AI Ассистент"),
						Rekogram::AIChatManager::Instance().enabled(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::AIChatManager::Instance().setEnabled(checked);
					Ui::Toast::Show(checked
						? QString::fromUtf8("AI Ассистент включен")
						: QString::fromUtf8("AI Ассистент отключен"));
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.addButton({
				.id = u"reko_ai/provider"_q,
				.title = rpl::single(QString::fromUtf8("Провайдер AI: ") + Rekogram::AIChatManager::Instance().provider()),
				.icon = { &st::menuIconEmojiObjects },
				.onClick = [=] {
					const auto current = Rekogram::AIChatManager::Instance().provider();
					const auto next = (current == u"Local (Offline)"_q)
						? u"Cloud API (Fast)"_q
						: u"Local (Offline)"_q;
					Rekogram::AIChatManager::Instance().setProvider(next);
					Ui::Toast::Show(QString::fromUtf8("Выбран AI провайдер: ") + next);
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 7. Plugins Section
class RekogramPluginsSection final : public Section<RekogramPluginsSection> {
public:
	RekogramPluginsSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Расширения и плагины"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Менеджер плагинов Rekogram")));
			builder.addButton({
				.id = u"reko_plug/list"_q,
				.title = rpl::single(QString::fromUtf8("Установленные расширения")),
				.icon = { &st::menuIconShowInFolder },
				.onClick = [=] {
					const auto plugins = Rekogram::PluginSystem::Instance().installedPlugins();
					QString listStr;
					for (const auto &p : plugins) {
						listStr += p.name + u" v"_q + p.version + u" - "_q + p.description + u"\n"_q;
					}
					controller->show(Ui::MakeConfirmBox({
						.text = listStr.isEmpty()
							? QString::fromUtf8("Установленных сторонних расширений нет.")
							: listStr,
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Плагины Rekogram"),
					}));
				},
			});
			builder.addButton({
				.id = u"reko_plug/reload"_q,
				.title = rpl::single(QString::fromUtf8("Перезагрузить манифесты плагинов")),
				.icon = { &st::menuIconRestore },
				.onClick = [=] {
					Rekogram::PluginSystem::Instance().reloadAll();
					Ui::Toast::Show(QString::fromUtf8("Манифесты плагинов обновлены"));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 8. Advanced Section
class RekogramAdvancedSection final : public Section<RekogramAdvancedSection> {
public:
	RekogramAdvancedSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Продвинутые настройки"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Лаборатория интерфейса")));
			builder.addButton({
				.id = u"reko_adv/shortcuts"_q,
				.title = rpl::single(QString::fromUtf8("Настройка горячих клавиш")),
				.icon = { &st::menuIconShortcut },
				.onClick = [=] { showOther(ShortcutsId()); },
			});
			builder.addButton({
				.id = u"reko_adv/palette"_q,
				.title = rpl::single(QString::fromUtf8("Палитра команд (Ctrl+Shift+P)")),
				.icon = { &st::menuIconManage },
				.onClick = [=] {
					Rekogram::CommandPalette::Show(controller);
				},
			});
			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Режим разработчика Rekogram"),
						Rekogram::SettingsManager::Instance().devMode(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::SettingsManager::Instance().setDevMode(checked);
					Ui::Toast::Show(checked
						? QString::fromUtf8("Режим разработчика активирован")
						: QString::fromUtf8("Режим разработчика отключен"));
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 9. Premium Section
class RekogramPremiumSection final : public Section<RekogramPremiumSection> {
public:
	RekogramPremiumSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("Rekogram Premium"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Rekogram Local Premium")));

			// Main Enable Local Premium toggle
			builder.add([=](const WidgetContext &ctx) {
				auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);
				const auto checkbox = wrap->add(
					object_ptr<Ui::Checkbox>(
						wrap.data(),
						QString::fromUtf8("Включить Rekogram Local Premium"),
						Rekogram::PremiumManager::Instance().localPremiumEnabled(),
						st::defaultCheckbox),
					st::settingsCheckboxPadding);

				checkbox->checkedChanges(
				) | rpl::on_next([=](bool checked) {
					Rekogram::PremiumManager::Instance().setLocalPremiumEnabled(checked);
					Ui::Toast::Show(checked
						? QString::fromUtf8("Rekogram Local Premium активирован!")
						: QString::fromUtf8("Rekogram Local Premium отключен"));
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			// Creator Badge toggle
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
				}, checkbox->lifetime());

				return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
			});

			builder.addSkip();
			builder.addDivider();
			builder.addSkip();

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Карта возможностей Local Premium")));

			builder.addButton({
				.id = u"reko_prem/features_map"_q,
				.title = rpl::single(QString::fromUtf8("Подробный статус возможностей (Available / Server)")),
				.icon = { &st::menuIconGiftPremium },
				.onClick = [=] {
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Статус возможностей Rekogram Local Premium:\n\n"
							"🟢 ДОСТУПНО ПОЛНОСТЬЮ (Available):\n"
							"• Разблокировка Custom Emoji Picker без Premium-окна\n"
							"• Лимит сохраненных GIF: 200 -> 400\n"
							"• Лимит избранных стикеров: 5 -> 10\n"
							"• Лимит папок диалогов: 10 -> 30\n"
							"• Лимит закрепленных чатов: 5 -> 10 (в папках до 200)\n"
							"• Пересортировка вкладок папок (включая «Все чаты»)\n"
							"• Лимит длины подписей к медиа: 1024 -> 2048\n"
							"• Лимит описания профиля Bio: 70 -> 140\n"
							"• Лимит длины сообщений: 4096 -> 8192\n"
							"• Воспроизведение анимированных видеоаватаров\n"
							"• Отдельный кастомный статус-бейдж Rekogram\n\n"
							"🟡 ЧАСТИЧНО ДОСТУПНО (Partially Available):\n"
							"• Анимированные эмодзи в сообщениях (рендеринг в чатах, отправка в Избранное/темы)\n"
							"• Анимированные стикеры и реакции (клиентские анимации)\n"
							"• Повышенная скорость загрузки (параллельные чанки)\n\n"
							"🔴 ТРЕБУЕТСЯ СЕРВЕР ТЕЛЕГРАМ (Server Required):\n"
							"• Отправка файлов до 4 ГБ (серверный лимит 2 ГБ)\n"
							"• Серверное распознавание голоса в текст\n"
							"• Серверный автоперевод каналов\n"
							"• Официальная звезда в официальных клиентах"),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Возможности Local Premium"),
					}));
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// 10. About Section
class RekogramAboutSection final : public Section<RekogramAboutSection> {
public:
	RekogramAboutSection(QWidget *parent, not_null<Window::SessionController*> controller)
	: Section(parent, controller) { setupContent(); }
	[[nodiscard]] rpl::producer<QString> title() override {
		return rpl::single(QString::fromUtf8("О Rekogram"));
	}
private:
	void setupContent() {
		const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
		const SectionBuildMethod buildMethod = [=](
				not_null<Ui::VerticalLayout*> container,
				not_null<Window::SessionController*> controller,
				Fn<void(Type)> showOther,
				rpl::producer<> showFinished) {
			auto &lifetime = container->lifetime();
			const auto highlights = lifetime.make_state<HighlightRegistry>();
			const auto isPaused = Window::PausedIn(controller, Window::GifPauseReason::Layer);
			auto builder = SectionBuilder(WidgetContext{
				.container = container,
				.controller = controller,
				.showOther = std::move(showOther),
				.isPaused = isPaused,
				.highlights = highlights,
			});

			builder.addSubsectionTitle(rpl::single(QString::fromUtf8("О приложении")));
			builder.addButton({
				.id = u"reko_abt/version"_q,
				.title = rpl::single(QString::fromUtf8("Версия: Rekogram v") + AppVersionStr),
				.icon = { &st::menuIconEmojiObjects },
				.onClick = [=] {
					controller->show(Ui::MakeConfirmBox({
						.text = QString::fromUtf8(
							"Rekogram Desktop — независимый клиент на базе Telegram Desktop.\n\n"
							"Версия: %1\n"
							"Создатель и ведущий архитектор: ID %2\n"
							"Лицензия: GNU GPL v3 с исключениями OpenSSL\n\n"
							"Проект развивает клиентские технологии приватности, кастомизации и локального премиума.")
							.arg(AppVersionStr)
							.arg(Rekogram::kDefaultOwnerUserId),
						.confirmText = QString::fromUtf8("Понятно"),
						.title = QString::fromUtf8("Rekogram Desktop"),
					}));
				},
			});
			builder.addButton({
				.id = u"reko_abt/github"_q,
				.title = rpl::single(QString::fromUtf8("Репозиторий на GitHub")),
				.icon = { &st::menuIconLinks },
				.onClick = [=] {
					UrlClickHandler::Open(u"https://github.com/vdyplesosiska2026-ux/Rekogram"_q);
				},
			});
		};
		build(content, buildMethod);
		Ui::ResizeFitChild(this, content);
	}
};

// Main Hub Section
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

		builder.addSubsectionTitle(rpl::single(QString::fromUtf8("Категории Rekogram Desktop")));

		// 1. General
		builder.addButton({
			.id = u"rekogram_nav/general"_q,
			.title = rpl::single(QString::fromUtf8("Общие настройки")),
			.icon = { &st::menuIconSettings },
			.onClick = [=] { showOther(RekogramGeneralId()); },
			.keywords = { u"general"_q, u"backup"_q, u"reset"_q, u"общие"_q },
		});

		// 2. Appearance
		builder.addButton({
			.id = u"rekogram_nav/appearance"_q,
			.title = rpl::single(QString::fromUtf8("Внешний вид и темы")),
			.icon = { &st::menuIconNightMode },
			.onClick = [=] { showOther(RekogramAppearanceId()); },
			.keywords = { u"appearance"_q, u"theme"_q, u"compact"_q, u"avatar"_q, u"внешний вид"_q },
		});

		// 3. Privacy & Security
		builder.addButton({
			.id = u"rekogram_nav/privacy"_q,
			.title = rpl::single(QString::fromUtf8("Приватность и безопасность")),
			.icon = { &st::menuIconLock },
			.onClick = [=] { showOther(RekogramPrivacyId()); },
			.keywords = { u"privacy"_q, u"security"_q, u"streamer"_q, u"shield"_q, u"приватность"_q },
		});

		// 4. Chats
		builder.addButton({
			.id = u"rekogram_nav/chats"_q,
			.title = rpl::single(QString::fromUtf8("Чаты и общение")),
			.icon = { &st::menuIconChatBubble },
			.onClick = [=] { showOther(RekogramChatsId()); },
			.keywords = { u"chats"_q, u"quick replies"_q, u"быстрые ответы"_q, u"шаблоны"_q },
		});

		// 5. Messages & Media
		builder.addButton({
			.id = u"rekogram_nav/messages"_q,
			.title = rpl::single(QString::fromUtf8("Сообщения и медиа")),
			.icon = { &st::menuIconShowInFolder },
			.onClick = [=] { showOther(RekogramMessagesId()); },
			.keywords = { u"messages"_q, u"inspector"_q, u"bookmarks"_q, u"reminders"_q, u"закладки"_q },
		});

		// 6. AI & Translation
		builder.addButton({
			.id = u"rekogram_nav/ai"_q,
			.title = rpl::single(QString::fromUtf8("AI и перевод")),
			.icon = { &st::menuIconTranslate },
			.onClick = [=] { showOther(RekogramAIId()); },
			.keywords = { u"ai"_q, u"translate"_q, u"assistant"_q, u"нейросеть"_q },
		});

		// 7. Extensions & Plugins
		builder.addButton({
			.id = u"rekogram_nav/plugins"_q,
			.title = rpl::single(QString::fromUtf8("Расширения и плагины")),
			.icon = { &st::menuIconManage },
			.onClick = [=] { showOther(RekogramPluginsId()); },
			.keywords = { u"plugins"_q, u"extensions"_q, u"плагины"_q },
		});

		// 8. Advanced
		builder.addButton({
			.id = u"rekogram_nav/advanced"_q,
			.title = rpl::single(QString::fromUtf8("Продвинутые настройки")),
			.icon = { &st::menuIconShortcut },
			.onClick = [=] { showOther(RekogramAdvancedId()); },
			.keywords = { u"advanced"_q, u"developer"_q, u"palette"_q, u"shortcuts"_q },
		});

		// 9. Rekogram Premium
		builder.addButton({
			.id = u"rekogram_nav/premium"_q,
			.title = rpl::single(QString::fromUtf8("Rekogram Premium")),
			.icon = { &st::menuIconGiftPremium },
			.onClick = [=] { showOther(RekogramPremiumId()); },
			.keywords = { u"premium"_q, u"local premium"_q, u"премиум"_q, u"emoji"_q },
		});

		// 10. About
		builder.addButton({
			.id = u"rekogram_nav/about"_q,
			.title = rpl::single(QString::fromUtf8("О Rekogram")),
			.icon = { &st::menuIconEmojiObjects },
			.onClick = [=] { showOther(RekogramAboutId()); },
			.keywords = { u"about"_q, u"version"_q, u"о программе"_q },
		});

		builder.addSkip();
	};

	build(content, buildMethod);
	Ui::ResizeFitChild(this, content);
}

// Meta registration for search and navigation
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
			.keywords = { u"rekogram"_q, u"custom"_q, u"appearance"_q, u"premium"_q },
		};
	});
});

const auto kMetaGeneral = BuildHelper({
	.id = RekogramGeneralSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconSettings,
}, [](SectionBuilder &builder) {});

const auto kMetaAppearance = BuildHelper({
	.id = RekogramAppearanceSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconNightMode,
}, [](SectionBuilder &builder) {});

const auto kMetaPrivacy = BuildHelper({
	.id = RekogramPrivacySection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconLock,
}, [](SectionBuilder &builder) {});

const auto kMetaChats = BuildHelper({
	.id = RekogramChatsSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconChatBubble,
}, [](SectionBuilder &builder) {});

const auto kMetaMessages = BuildHelper({
	.id = RekogramMessagesSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconShowInFolder,
}, [](SectionBuilder &builder) {});

const auto kMetaAI = BuildHelper({
	.id = RekogramAISection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconTranslate,
}, [](SectionBuilder &builder) {});

const auto kMetaPlugins = BuildHelper({
	.id = RekogramPluginsSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconManage,
}, [](SectionBuilder &builder) {});

const auto kMetaAdvanced = BuildHelper({
	.id = RekogramAdvancedSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconShortcut,
}, [](SectionBuilder &builder) {});

const auto kMetaPremium = BuildHelper({
	.id = RekogramPremiumSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconGiftPremium,
}, [](SectionBuilder &builder) {});

const auto kMetaAbout = BuildHelper({
	.id = RekogramAboutSection::Id(),
	.parentId = RekogramSection::Id(),
	.title = &tr::lng_settings_section_info,
	.icon = &st::menuIconEmojiObjects,
}, [](SectionBuilder &builder) {});

} // namespace

Type RekogramId() { return RekogramSection::Id(); }
Type RekogramGeneralId() { return RekogramGeneralSection::Id(); }
Type RekogramAppearanceId() { return RekogramAppearanceSection::Id(); }
Type RekogramPrivacyId() { return RekogramPrivacySection::Id(); }
Type RekogramChatsId() { return RekogramChatsSection::Id(); }
Type RekogramMessagesId() { return RekogramMessagesSection::Id(); }
Type RekogramAIId() { return RekogramAISection::Id(); }
Type RekogramPluginsId() { return RekogramPluginsSection::Id(); }
Type RekogramAdvancedId() { return RekogramAdvancedSection::Id(); }
Type RekogramPremiumId() { return RekogramPremiumSection::Id(); }
Type RekogramAboutId() { return RekogramAboutSection::Id(); }

} // namespace Settings
