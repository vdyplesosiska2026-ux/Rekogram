/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QJsonObject>

namespace Window {
class SessionController;
} // namespace Window

namespace Rekogram {
namespace Advanced {

struct SystemDiagnostics {
	QString osVersion;
	QString compilerVersion;
	QString qtVersion;
	QString buildArch;
	QString buildDate;
	QString activeDC;
	quint64 memoryUsageBytes = 0;
	QString gitCommitHash;
};

class AdvancedToolsManager final {
public:
	static AdvancedToolsManager &Instance();

	// Feature 23: Message Raw MTProto & JSON Inspector
	[[nodiscard]] QJsonObject inspectMessageToJson(
		uint64 peerId,
		int64 msgId,
		int64 date,
		int64 editDate,
		uint64 fromPeerId,
		bool isOut,
		int views,
		int forwards,
		const QString &text) const;

	// Feature 24: One-Click Cache Cleaner & Database Vacuum
	struct CacheCleanResult {
		int filesRemoved = 0;
		quint64 bytesFreed = 0;
		bool success = true;
	};
	[[nodiscard]] CacheCleanResult cleanLocalCache() const;

	// Feature 25: Team Role Switcher Helper
	[[nodiscard]] QStringList availableRoles() const;
	[[nodiscard]] QString currentRole() const;
	void cycleNextRole();

	// Feature 26: Active Telegram Data Center (DC) Monitor
	[[nodiscard]] QString currentDCName(int dcId) const;

	// Feature 27: Export Chat to Markdown Helper
	[[nodiscard]] QString exportToMarkdown(
		const QString &chatTitle,
		const std::vector<std::pair<QString, QString>> &messages) const;

	// Feature 28: Custom Startup Chat / Folder
	[[nodiscard]] QString defaultStartupSection() const { return _defaultStartupSection; }
	void setDefaultStartupSection(const QString &section);

	// Feature 29: Fast Account Switcher Hotkeys Enabled
	[[nodiscard]] bool accountHotkeysEnabled() const { return _accountHotkeysEnabled; }
	void setAccountHotkeysEnabled(bool enabled);

	// Feature 30: System Diagnostics
	[[nodiscard]] SystemDiagnostics diagnostics() const;

	void resetToDefaults();

private:
	AdvancedToolsManager();
	void load();
	void save();

	QString _defaultStartupSection = u"dialogs"_q;
	bool _accountHotkeysEnabled = true;
};

} // namespace Advanced
} // namespace Rekogram
