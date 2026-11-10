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

	[[nodiscard]] bool enabled() const { return _enabled; }
	void setEnabled(bool val) { _enabled = val; }
	[[nodiscard]] QString provider() const { return _provider.isEmpty() ? providerName() : _provider; }
	void setProvider(const QString &val) { _provider = val; }

	void sendMessage(const QString &text);
	[[nodiscard]] std::vector<AIChatMessage> messages() const;
	void clearHistory();
	void reload() { load(); }

	[[nodiscard]] bool isConfigured() const;
	[[nodiscard]] QString providerName() const;

private:
	AIChatManager();
	void load();
	void save();

	bool _enabled = true;
	QString _provider = QString::fromUtf8("Local (Offline)");
	std::vector<AIChatMessage> _messages;
};

} // namespace Rekogram
