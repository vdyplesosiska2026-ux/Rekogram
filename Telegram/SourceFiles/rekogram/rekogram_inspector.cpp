/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_inspector.h"

#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_components.h"
#include "ui/boxes/confirm_box.h"
#include "window/window_session_controller.h"

namespace Rekogram {

MessageDetails MessageInspector::ExtractDetails(not_null<HistoryItem*> item) {
	MessageDetails details;
	details.fullId = item->fullId();
	details.fromPeerId = item->from() ? item->from()->id : PeerId(0);
	details.toPeerId = item->history() ? item->history()->peer->id : PeerId(0);
	details.date = QDateTime::fromSecsSinceEpoch(item->date());
	details.rawText = item->originalText().text;
	return details;
}

void MessageInspector::Inspect(not_null<Window::SessionController*> controller, not_null<HistoryItem*> item) {
	const auto details = ExtractDetails(item);

	const auto content = QString::fromUtf8(
		"ID сообщения: %1\n"
		"Peer ID чата: %2\n"
		"Отправитель: %3\n"
		"Дата отправки: %4\n"
		"Символов в тексте: %5\n"
		"Исходящее: %6\n\n"
		"Содержимое:\n%7")
		.arg(details.fullId.msg.bare)
		.arg(details.toPeerId.value)
		.arg(details.fromPeerId.value)
		.arg(details.date.toString("yyyy-MM-dd HH:mm:ss"))
		.arg(details.rawText.size())
		.arg(item->out() ? QString::fromUtf8("Да") : QString::fromUtf8("Нет"))
		.arg(details.rawText.left(500));

	controller->show(Ui::MakeConfirmBox({
		.text = content,
		.confirmText = QString::fromUtf8("Закрыть"),
		.title = QString::fromUtf8("Инспектор сообщения (Message Inspector)"),
	}));
}

} // namespace Rekogram
