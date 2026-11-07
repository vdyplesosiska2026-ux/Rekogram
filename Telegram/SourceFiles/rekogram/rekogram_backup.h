/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace Rekogram {

class BackupManager final {
public:
	static QByteArray ExportSettings();
	static bool ImportSettings(const QByteArray &data, QString *outError = nullptr);
};

} // namespace Rekogram
