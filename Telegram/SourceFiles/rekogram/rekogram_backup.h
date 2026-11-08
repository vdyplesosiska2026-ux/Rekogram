/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

namespace Rekogram {

class BackupManager final {
public:
	static BackupManager &Instance() {
		static BackupManager inst;
		return inst;
	}

	static QByteArray ExportSettings();
	static bool ImportSettings(const QByteArray &data, QString *outError = nullptr);

	void exportToJson() const {
		const auto data = ExportSettings();
		QGuiApplication::clipboard()->setText(QString::fromUtf8(data));
	}

	bool importFromJson(const QString &json) const {
		return ImportSettings(json.toUtf8());
	}
};

} // namespace Rekogram
