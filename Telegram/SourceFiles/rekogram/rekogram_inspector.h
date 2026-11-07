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

namespace Window {
class SessionController;
} // namespace Window

class HistoryItem;

namespace Rekogram {

struct MessageDetails {
	FullMsgId fullId;
	PeerId fromPeerId = 0;
	PeerId toPeerId = 0;
	QDateTime date;
	QDateTime editDate;
	int mediaType = 0;
	int forwardOriginalDate = 0;
	QString rawText;
};

class MessageInspector final {
public:
	static void Inspect(not_null<Window::SessionController*> controller, not_null<HistoryItem*> item);
	static MessageDetails ExtractDetails(not_null<HistoryItem*> item);
};

} // namespace Rekogram
