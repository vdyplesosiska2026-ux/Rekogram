/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QByteArray>
#include <vector>

namespace Rekogram {

struct AIChatMessage {
	QString id;
	QString text;
	bool isOutgoing = false;
	QDateTime timestamp;
};

class AIChatManager final {
public:
	static AIChatManager &Instance();

	void sendMessage(const QString &text);
	[[nodiscard]] std::vector<AIChatMessage> messages() const;
	void clearHistory();

	[[nodiscard]] bool isConfigured() const;
	[[nodiscard]] QString providerName() const;

private:
	AIChatManager();
	void load();
	void save();

	std::vector<AIChatMessage> _messages;
};

} // namespace Rekogram
