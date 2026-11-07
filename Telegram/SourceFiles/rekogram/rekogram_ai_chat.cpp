/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_ai_chat.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

namespace Rekogram {
namespace {

constexpr auto kAIChatHistoryPref = "reko_ai_chat_history";

} // namespace

AIChatManager &AIChatManager::Instance() {
	static AIChatManager instance;
	return instance;
}

AIChatManager::AIChatManager() {
	load();
}

void AIChatManager::load() {
	_messages.clear();
	const auto raw = Core::App().settings().readPref<QByteArray>(kAIChatHistoryPref, QByteArray());
	if (raw.isEmpty()) {
		return;
	}

	const auto doc = QJsonDocument::fromJson(raw);
	if (!doc.isArray()) {
		return;
	}

	const auto arr = doc.array();
	_messages.reserve(arr.size());
	for (const auto &val : arr) {
		if (!val.isObject()) continue;
		const auto obj = val.toObject();
		AIChatMessage msg;
		msg.id = obj["id"].toString();
		msg.text = obj["text"].toString();
		msg.isOutgoing = obj["out"].toBool(false);
		msg.timestamp = QDateTime::fromString(obj["time"].toString(), Qt::ISODate);
		_messages.push_back(std::move(msg));
	}
}

void AIChatManager::save() {
	QJsonArray arr;
	for (const auto &msg : _messages) {
		QJsonObject obj;
		obj["id"] = msg.id;
		obj["text"] = msg.text;
		obj["out"] = msg.isOutgoing;
		obj["time"] = msg.timestamp.toString(Qt::ISODate);
		arr.append(obj);
	}
	const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Compact);
	Core::App().settings().writePref<QByteArray>(kAIChatHistoryPref, bytes);
}

void AIChatManager::sendMessage(const QString &text) {
	if (text.trimmed().isEmpty()) return;

	AIChatMessage userMsg;
	userMsg.id = QString::number(QDateTime::currentMSecsSinceEpoch());
	userMsg.text = text;
	userMsg.isOutgoing = true;
	userMsg.timestamp = QDateTime::currentDateTime();
	_messages.push_back(std::move(userMsg));

	// System notice when AI is not configured yet
	if (!isConfigured()) {
		AIChatMessage systemNotice;
		systemNotice.id = QString::number(QDateTime::currentMSecsSinceEpoch() + 1);
		systemNotice.text = QString::fromUtf8(
			"Rekogram AI пока находится в режиме подготовки архитектуры.\n\n"
			"Интерфейс чата готов к работе, сетевой стек и API-провайдеры будут подключены на следующем этапе разработки.");
		systemNotice.isOutgoing = false;
		systemNotice.timestamp = QDateTime::currentDateTime();
		_messages.push_back(std::move(systemNotice));
	}

	save();
}

std::vector<AIChatMessage> AIChatManager::messages() const {
	return _messages;
}

void AIChatManager::clearHistory() {
	_messages.clear();
	save();
}

bool AIChatManager::isConfigured() const {
	// Not configured yet - will allow user API keys or custom endpoints in future updates.
	return false;
}

QString AIChatManager::providerName() const {
	return QString::fromUtf8("Rekogram AI Engine (Offline Architecture)");
}

} // namespace Rekogram
