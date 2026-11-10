/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_bookmarks_ui.h"

#include "rekogram/rekogram_bookmarks.h"
#include "rekogram/rekogram_reminders.h"
#include "data/data_peer_id.h"
#include "data/data_msg_id.h"
#include "window/window_session_controller.h"
#include "mainwindow.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QDateTime>

namespace Rekogram {
namespace BookmarksUI {
namespace {

class BookmarksDialog final : public QDialog {
public:
	BookmarksDialog(not_null<Window::SessionController*> controller)
	: QDialog(controller->widget()) {
		setWindowTitle(QString::fromUtf8("Закладки и напоминания Rekogram"));
		setModal(true);
		resize(560, 480);

		auto *layout = new QVBoxLayout(this);
		layout->setContentsMargins(12, 12, 12, 12);
		layout->setSpacing(8);

		_tabs = new QTabWidget(this);
		layout->addWidget(_tabs, 1);

		_bookmarksList = new QListWidget(this);
		_tabs->addTab(_bookmarksList, QString::fromUtf8("Закладки"));

		_remindersList = new QListWidget(this);
		_tabs->addTab(_remindersList, QString::fromUtf8("Напоминания"));

		auto *buttons = new QHBoxLayout;
		buttons->addStretch(1);

		_removeBtn = new QPushButton(QString::fromUtf8("Удалить"), this);
		connect(_removeBtn, &QPushButton::clicked, this, &BookmarksDialog::removeSelected);
		buttons->addWidget(_removeBtn);

		_closeBtn = new QPushButton(QString::fromUtf8("Закрыть"), this);
		connect(_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
		buttons->addWidget(_closeBtn);
		layout->addLayout(buttons);

		connect(_tabs, &QTabWidget::currentChanged, this, &BookmarksDialog::refreshList);
		connect(_bookmarksList, &QListWidget::itemSelectionChanged, this, &BookmarksDialog::updateRemoveState);
		connect(_remindersList, &QListWidget::itemSelectionChanged, this, &BookmarksDialog::updateRemoveState);

		refreshList(0);
	}

private:
	void refreshList(int tab) {
		if (tab == 0) {
			_bookmarksList->clear();
			const auto items = BookmarksManager::Instance().list();
			for (const auto &item : items) {
				auto *li = new QListWidgetItem(_bookmarksList);
				QString text = item.previewText;
				if (text.size() > 120) text = text.left(120) + "…";
				li->setText(text);
				li->setData(Qt::UserRole, QString::number(item.msgId.peer.value) + ":" + QString::number(item.msgId.msg.bare));
				if (!item.note.isEmpty()) {
					li->setToolTip(item.note);
				}
			}
		} else {
			_remindersList->clear();
			const auto items = RemindersManager::Instance().activeReminders();
			for (const auto &item : items) {
				auto *li = new QListWidgetItem(_remindersList);
				QString text = item.text.isEmpty() ? QString::fromUtf8("(без текста)") : item.text;
				if (text.size() > 100) text = text.left(100) + "…";
				li->setText(QString::fromUtf8("%1 — %2")
					.arg(text)
					.arg(item.triggerTime.toString("dd.MM.yyyy HH:mm")));
				li->setData(Qt::UserRole, QString::number(item.msgId.peer.value) + ":" + QString::number(item.msgId.msg.bare));
			}
		}
		updateRemoveState();
	}

	void updateRemoveState() {
		const auto tab = _tabs->currentIndex();
		const auto *list = (tab == 0) ? _bookmarksList : _remindersList;
		_removeBtn->setEnabled(!list->selectedItems().isEmpty());
	}

	void removeSelected() {
		const auto tab = _tabs->currentIndex();
		if (tab == 0) {
			const auto items = _bookmarksList->selectedItems();
			for (const auto *item : items) {
				const auto parts = item->data(Qt::UserRole).toString().split(':');
				if (parts.size() == 2) {
					FullMsgId id;
					id.peer = PeerId(PeerIdHelper{ parts[0].toULongLong() });
					id.msg = MsgId(parts[1].toLongLong());
					BookmarksManager::Instance().removeBookmark(id);
				}
			}
		} else {
			const auto items = _remindersList->selectedItems();
			for (const auto *item : items) {
				const auto parts = item->data(Qt::UserRole).toString().split(':');
				if (parts.size() == 2) {
					FullMsgId id;
					id.peer = PeerId(PeerIdHelper{ parts[0].toULongLong() });
					id.msg = MsgId(parts[1].toLongLong());
					RemindersManager::Instance().removeReminder(id);
				}
			}
		}
		refreshList(tab);
	}

	QTabWidget *_tabs = nullptr;
	QListWidget *_bookmarksList = nullptr;
	QListWidget *_remindersList = nullptr;
	QPushButton *_removeBtn = nullptr;
	QPushButton *_closeBtn = nullptr;
};

} // namespace

void Show(not_null<Window::SessionController*> controller) {
	BookmarksDialog dialog(controller);
	dialog.exec();
}

} // namespace BookmarksUI
} // namespace Rekogram
