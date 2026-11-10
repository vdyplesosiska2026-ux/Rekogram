/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_advanced_tools.h"
#include "rekogram/rekogram_settings.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "core/version.h"
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSysInfo>
#include <QtCore/QDateTime>

namespace Rekogram {
namespace Advanced {
namespace {

constexpr auto kDefaultStartupPref = "reko_adv_startup_section";
constexpr auto kAccountHotkeysPref = "reko_adv_account_hotkeys";

} // namespace

AdvancedToolsManager &AdvancedToolsManager::Instance() {
	static AdvancedToolsManager instance;
	return instance;
}

AdvancedToolsManager::AdvancedToolsManager() {
	load();
}

void AdvancedToolsManager::load() {
	auto &core = Core::App().settings();
	_defaultStartupSection = QString::fromUtf8(core.readPref<QByteArray>(kDefaultStartupPref, "dialogs"));
	_accountHotkeysEnabled = core.readPref<bool>(kAccountHotkeysPref, true);
}

void AdvancedToolsManager::save() {
	auto &core = Core::App().settings();
	core.writePref<QByteArray>(kDefaultStartupPref, _defaultStartupSection.toUtf8());
	core.writePref<bool>(kAccountHotkeysPref, _accountHotkeysEnabled);
}

QJsonObject AdvancedToolsManager::inspectMessageToJson(
		uint64 peerId,
		int64 msgId,
		int64 date,
		int64 editDate,
		uint64 fromPeerId,
		bool isOut,
		int views,
		int forwards,
		const QString &text) const {
	QJsonObject root;
	root["peer_id"] = QString::number(peerId);
	root["msg_id"] = QString::number(msgId);
	root["date_unix"] = date;
	root["date_iso"] = QDateTime::fromSecsSinceEpoch(date).toString(Qt::ISODate);
	if (editDate > 0) {
		root["edit_date_unix"] = editDate;
		root["edit_date_iso"] = QDateTime::fromSecsSinceEpoch(editDate).toString(Qt::ISODate);
	}
	root["from_peer_id"] = QString::number(fromPeerId);
	root["is_outgoing"] = isOut;
	root["views_count"] = views;
	root["forwards_count"] = forwards;
	root["content_text"] = text;
	root["char_length"] = text.size();
	root["utf8_bytes"] = text.toUtf8().size();
	return root;
}

AdvancedToolsManager::CacheCleanResult AdvancedToolsManager::cleanLocalCache() const {
	CacheCleanResult res;
	const auto working = cWorkingDir();
	const auto tempDir = QDir(working + "tdata/temp");
	if (tempDir.exists()) {
		const auto entries = tempDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
		for (const auto &info : entries) {
			res.bytesFreed += info.size();
			if (QFile::remove(info.absoluteFilePath())) {
				res.filesRemoved++;
			}
		}
	}
	return res;
}

QStringList AdvancedToolsManager::availableRoles() const {
	return {
		QString::fromUtf8("Rekogram Архитектор"),
		QString::fromUtf8("Rekogram Developer"),
		QString::fromUtf8("Создатель Rekogram"),
		QString::fromUtf8("Rekogram Lead Engineer"),
		QString::fromUtf8("Rekogram UI/UX Designer"),
	};
}

QString AdvancedToolsManager::currentRole() const {
	return SettingsManager::Instance().creatorRole();
}

void AdvancedToolsManager::cycleNextRole() {
	SettingsManager::Instance().nextCreatorRole();
}

QString AdvancedToolsManager::currentDCName(int dcId) const {
	switch (dcId) {
	case 1: return QString::fromUtf8("DC1 — Miami, USA (149.154.175.50)");
	case 2: return QString::fromUtf8("DC2 — Amsterdam, NL (149.154.167.50)");
	case 3: return QString::fromUtf8("DC3 — Miami, USA (149.154.175.100)");
	case 4: return QString::fromUtf8("DC4 — Amsterdam, NL (149.154.167.91)");
	case 5: return QString::fromUtf8("DC5 — Singapore, SG (91.108.56.165)");
	default: return QString::fromUtf8("DC%1 — Telegram Edge").arg(dcId);
	}
}

QString AdvancedToolsManager::exportToMarkdown(
		const QString &chatTitle,
		const std::vector<std::pair<QString, QString>> &messages) const {
	QString md;
	md += u"# Чат: "_q + chatTitle + u"\n"_q;
	md += u"*Экспортировано через Rekogram Desktop: "_q + QDateTime::currentDateTime().toString(Qt::ISODate) + u"*\n\n---\n\n"_q;
	for (const auto &[author, content] : messages) {
		md += u"### "_q + author + u"\n"_q;
		md += content + u"\n\n"_q;
	}
	return md;
}

void AdvancedToolsManager::setDefaultStartupSection(const QString &section) {
	if (_defaultStartupSection != section && !section.isEmpty()) {
		_defaultStartupSection = section;
		save();
	}
}

void AdvancedToolsManager::setAccountHotkeysEnabled(bool enabled) {
	if (_accountHotkeysEnabled != enabled) {
		_accountHotkeysEnabled = enabled;
		save();
	}
}

SystemDiagnostics AdvancedToolsManager::diagnostics() const {
	SystemDiagnostics diag;
	diag.osVersion = QSysInfo::prettyProductName() + u" ("_q + QSysInfo::currentCpuArchitecture() + u")"_q;
#if defined(_MSC_VER)
	diag.compilerVersion = QString::fromUtf8("MSVC %1").arg(_MSC_VER);
#elif defined(__clang__)
	diag.compilerVersion = QString::fromUtf8("Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__);
#elif defined(__GNUC__)
	diag.compilerVersion = QString::fromUtf8("GCC %1.%2").arg(__GNUC__).arg(__GNUC_MINOR__);
#else
	diag.compilerVersion = u"Unknown Compiler"_q;
#endif
	diag.qtVersion = QString::fromUtf8(QT_VERSION_STR);
	diag.buildArch = QSysInfo::buildCpuArchitecture();
	diag.buildDate = QString::fromUtf8(__DATE__) + u" "_q + QString::fromUtf8(__TIME__);
	diag.gitCommitHash = u"56c1489"_q;
	diag.activeDC = currentDCName(2); // DC2 is default primary European/CIS DC
	diag.memoryUsageBytes = 0;
	return diag;
}

void AdvancedToolsManager::resetToDefaults() {
	_defaultStartupSection = u"dialogs"_q;
	_accountHotkeysEnabled = true;
	save();
}

} // namespace Advanced
} // namespace Rekogram
