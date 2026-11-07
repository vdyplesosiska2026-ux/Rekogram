<div align="center">

# Rekogram Desktop

**Современный, быстрый и функциональный форк Telegram Desktop**  
*Сохраняющий оригинальный дух, дизайн и стандарты безопасности Telegram Desktop.*

[![Release](https://img.shields.io/badge/version-1.0.0--dev-blue.svg?style=for-the-badge)](https://github.com/vdyplesosiska2026-ux/Rekogram/releases)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg?style=for-the-badge)](https://github.com/vdyplesosiska2026-ux/Rekogram)
[![License](https://img.shields.io/badge/license-GNU%20GPL%20v3-green.svg?style=for-the-badge)](LICENSE)
[![Base](https://img.shields.io/badge/based%20on-Telegram%20Desktop%207.4.1-blueviolet.svg?style=for-the-badge)](https://github.com/telegramdesktop/tdesktop)

[Возможности](#-основные-возможности-rekogram) • [Стек технологий](#-технологический-стек) • [Сборка](#-инструкция-по-сборке) • [Архитектура](#-архитектура-проекта) • [Лицензия](#-лицензия-и-авторские-права)

---

</div>

## 📖 О проекте

**Rekogram** — это независимый форк официального клиента [Telegram Desktop](https://github.com/telegramdesktop/tdesktop), созданный для энтузиастов и пользователей, которым необходим расширенный контроль над внешним видом, закладками, напоминаниями и инструментами персонализации без искажения оригинальной эстетики и без перегрузки интерфейса.

### Ключевые принципы разработки:
- **Сохранение дизайна Telegram Desktop**: никаких чуждых стилей из Slack/Discord/WhatsApp — все элементы оформлены в соответствии с нативной дизайн-системой Telegram.
- **Стабильность MTProto**: не затрагиваются криптографические протоколы, работа сессий и безопасность данных.
- **Отсутствие декоративных заглушек**: каждая кнопка и функция в коде подкреплена реальной логикой.

---

## ⚡ Собственные возможности Rekogram

| Модуль | Описание и статус |
| :--- | :--- |
| **Rekogram Settings** | Отдельный изолированный экран настроек, вызываемый из главного меню и визуально отделённый от стандартных опций. |
| **Rekogram Themes & Night Mode** | Быстрое управление тёмной/светлой темой, выбор фона для чатов и безопасный сброс параметров оформления. |
| **Creator Badge** | Централизованная система проверки владельца по `OWNER_ID` и отрисовка аккуратного значка создателя слева от имени пользователя. |
| **Advanced Chat List** | Поддержка компактного режима отображения списка диалогов и сохранение пользовательских предпочтений. |
| **Rekogram Bookmarks** | Локальная система сохранения избранных сообщений в закладки через нативное контекстное меню любого сообщения. |
| **Message Reminders** | Встроенный планировщик напоминаний на сообщения с установкой времени срабатывания прямо из контекстного меню. |
| **Rekogram Shortcuts** | Интеграция с системой настройки клавиатурных комбинаций и горячих клавиш. |
| **Settings Backup** | Экспорт и импорт параметров конфигурации Rekogram в формате JSON с валидацией версий и защитой от повреждения. |
| **Interface Lab** | Переключатель экспериментальных возможностей для разработчиков и тестеров. |
| **Rekogram AI Chat** | Архитектура отдельного диалога Rekogram AI, выполненного в виде стандартного чата Telegram (демо-режим без сторонних SDK). |
| **Rekogram Hub** | Информационный центр с версией клиента, юридической информацией, ссылкой на исходный репозиторий и лицензиями. |

---

## 🛠 Технологический стек

- **Язык программирования**: C++20
- **UI & Графика**: Qt 6.x / Qt 5.15 (патченная ветка `desktop-app`), аппаратно-ускоренный рендеринг QRhi / OpenGL / DirectX
- **Сетевой стек**: Реализация MTProto 2.0 Telegram
- **Система сборки**: CMake 3.16+, Ninja / MSVC 2022
- **Хранилище параметров**: Сериализованные ключи настроек с обратной совместимостью

---

## 📂 Архитектура проекта

```text
Rekogram/
├── Telegram/
│   ├── SourceFiles/
│   │   ├── rekogram/                  # Специализированные модули Rekogram
│   │   │   ├── rekogram_settings.h/cpp # Конфигурация и OWNER_ID
│   │   │   ├── rekogram_badge.h/cpp    # Векторный Creator Badge
│   │   │   ├── rekogram_bookmarks.h/cpp# Система закладок сообщений
│   │   │   ├── rekogram_reminders.h/cpp# Менеджер напоминаний
│   │   │   ├── rekogram_backup.h/cpp   # Экспорт/импорт конфигурации
│   │   │   └── rekogram_ai_chat.h/cpp  # Архитектура Rekogram AI
│   │   ├── settings/
│   │   │   └── sections/
│   │   │       ├── settings_rekogram.h/cpp # Экран «Настройки Rekogram»
│   │   │       └── settings_main.cpp       # Кнопка входа в раздел
│   │   └── history/view/
│   │       ├── history_view_context_menu.cpp # Меню закладок и напоминаний
│   │       └── history_view_top_bar_widget.cpp # Отображение Creator Badge
├── CHANGELOG.md                       # История изменений
├── CONTRIBUTING.md                    # Правила разработки
└── README.md                          # Главная страница репозитория
```

---

## 🔨 Инструкция по сборке

### Windows (Visual Studio 2022)
1. Установите **Visual Studio 2022** с компонентами C++ Desktop Development.
2. Подготовьте зависимые библиотеки в соседнем каталоге (согласно структуре Telegram Desktop `win64/Libraries`).
3. Запустите консоль **x64 Native Tools Command Prompt for VS 2022**.
4. Сконфигурируйте и соберите проект:
   ```cmd
   cmake -B out -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
   cmake --build out --config Debug --target Telegram
   ```

---

## 📄 Лицензия и авторские права

Rekogram распространяется под свободной лицензией **GNU General Public License v3.0 with OpenSSL Exception**.  
Полный текст лицензии доступен в файле [LICENSE](LICENSE).

- **Telegram Desktop**: Copyright (c) 2014-2026 Telegram Desktop authors.
- **Rekogram**: Развивается как открытый форк сообщества.
