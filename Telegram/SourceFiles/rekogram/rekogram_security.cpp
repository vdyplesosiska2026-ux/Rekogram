/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_security.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "ui/toast/toast.h"
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

namespace Rekogram {
namespace Security {
namespace {

constexpr auto kAntiPhishingPref = "reko_sec_anti_phishing";
constexpr auto kStrictP2PPref = "reko_sec_strict_p2p";
constexpr auto kStripMetadataPref = "reko_sec_strip_metadata";
constexpr auto kAutoClearSearchPref = "reko_sec_auto_clear_search";
constexpr auto kBlurSensitivePref = "reko_sec_blur_sensitive";

// Dangerous Windows/system protocol handlers exploited in link attacks
const QStringList kDangerousSchemes = {
	u"ms-msdt"_q,
	u"search-ms"_q,
	u"ms-appinstaller"_q,
	u"javascript"_q,
	u"vbscript"_q,
	u"data"_q,
	u"res"_q,
	u"file"_q,
	u"shell"_q,
	u"cmd"_q,
	u"powershell"_q,
	u"diskpart"_q,
	u"rundll32"_q,
};

// Target domains frequently spoofed via IDN homographs
const QStringList kProtectedBrandDomains = {
	u"telegram.org"_q,
	u"t.me"_q,
	u"telegram.me"_q,
	u"fragment.com"_q,
	u"google.com"_q,
	u"github.com"_q,
	u"steamcommunity.com"_q,
	u"discord.com"_q,
	u"binance.com"_q,
	u"ton.org"_q,
};

bool HasHomographMixedScript(const QString &host) {
	if (host.startsWith(u"xn--"_q, Qt::CaseInsensitive)) {
		return true; // Punycode domain
	}
	// Check for Cyrillic / Greek characters embedded inside Latin domain
	bool hasLatin = false;
	bool hasCyrillicOrConfusable = false;
	for (const auto ch : host) {
		const auto u = ch.unicode();
		if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '-' || u == '.') {
			hasLatin = true;
		} else if ((u >= 0x0400 && u <= 0x04FF) || (u >= 0x0370 && u <= 0x03FF)) {
			hasCyrillicOrConfusable = true;
		}
	}
	return hasLatin && hasCyrillicOrConfusable;
}

} // namespace

SecurityManager &SecurityManager::Instance() {
	static SecurityManager instance;
	return instance;
}

SecurityManager::SecurityManager() {
	load();
}

void SecurityManager::load() {
	auto &core = Core::App().settings();
	_antiPhishingEnabled = core.readPref<bool>(kAntiPhishingPref, true);
	_strictP2PRelay = core.readPref<bool>(kStrictP2PPref, true);
	_stripMediaMetadata = core.readPref<bool>(kStripMetadataPref, true);
	_autoClearSearchOnLock = core.readPref<bool>(kAutoClearSearchPref, true);
	_blurSensitiveMedia = core.readPref<bool>(kBlurSensitivePref, false);
}

void SecurityManager::save() {
	auto &core = Core::App().settings();
	core.writePref<bool>(kAntiPhishingPref, _antiPhishingEnabled);
	core.writePref<bool>(kStrictP2PPref, _strictP2PRelay);
	core.writePref<bool>(kStripMetadataPref, _stripMediaMetadata);
	core.writePref<bool>(kAutoClearSearchPref, _autoClearSearchOnLock);
	core.writePref<bool>(kBlurSensitivePref, _blurSensitiveMedia);
}

void SecurityManager::setAntiPhishingEnabled(bool enabled) {
	if (_antiPhishingEnabled != enabled) {
		_antiPhishingEnabled = enabled;
		save();
	}
}

void SecurityManager::setStrictP2PRelay(bool enabled) {
	if (_strictP2PRelay != enabled) {
		_strictP2PRelay = enabled;
		save();
	}
}

void SecurityManager::setStripMediaMetadata(bool enabled) {
	if (_stripMediaMetadata != enabled) {
		_stripMediaMetadata = enabled;
		save();
	}
}

void SecurityManager::setAutoClearSearchOnLock(bool enabled) {
	if (_autoClearSearchOnLock != enabled) {
		_autoClearSearchOnLock = enabled;
		save();
	}
}

void SecurityManager::setBlurSensitiveMedia(bool enabled) {
	if (_blurSensitiveMedia != enabled) {
		_blurSensitiveMedia = enabled;
		save();
	}
}

UrlAuditDetails SecurityManager::auditUrl(const QString &url) const {
	UrlAuditDetails details;
	const auto trimmed = url.trimmed();
	const auto qurl = QUrl(trimmed);
	details.scheme = qurl.scheme().toLower();
	details.normalizedHost = qurl.host().toLower();

	// Check dangerous protocol handlers
	for (const auto &badScheme : kDangerousSchemes) {
		if (details.scheme == badScheme || trimmed.startsWith(badScheme + QStringLiteral(":"), Qt::CaseInsensitive)) {
			details.result = UrlSafetyResult::DangerousScheme;
			details.isDangerous = true;
			details.warningMessage = QString::fromUtf8("Заблокирован опасный системный протокол: ") + badScheme;
			return details;
		}
	}

	// Check credential spoofing (e.g. https://google.com@evil.com)
	if (!qurl.userInfo().isEmpty()) {
		details.result = UrlSafetyResult::CredentialSpoofing;
		details.isDangerous = true;
		details.warningMessage = QString::fromUtf8("Обнаружена попытка маскировки URL через встроенные учётные данные.");
		return details;
	}

	// Check IDN Homograph attack if anti-phishing is enabled
	if (_antiPhishingEnabled && HasHomographMixedScript(details.normalizedHost)) {
		details.result = UrlSafetyResult::SuspiciousHomograph;
		details.isDangerous = true;
		details.warningMessage = QString::fromUtf8("Внимание: адрес использует похожие символы разных алфавитов (IDN Homograph Phishing): ") + details.normalizedHost;
		return details;
	}

	details.result = UrlSafetyResult::Safe;
	return details;
}

bool SecurityManager::isUrlSafe(const QString &url) const {
	const auto audit = auditUrl(url);
	return !audit.isDangerous;
}

QByteArray SecurityManager::stripExifFromJpeg(const QByteArray &jpegData) const {
	if (jpegData.size() < 4) return jpegData;
	// Verify JPEG SOI marker (0xFF, 0xD8)
	const auto data = reinterpret_cast<const uint8_t*>(jpegData.constData());
	if (data[0] != 0xFF || data[1] != 0xD8) {
		return jpegData; // Not a JPEG, return as-is
	}

	QByteArray clean;
	clean.reserve(jpegData.size());
	clean.append(reinterpret_cast<const char*>(data), 2); // SOI

	int pos = 2;
	const int len = jpegData.size();
	while (pos + 4 <= len) {
		if (data[pos] != 0xFF) break;
		const uint8_t marker = data[pos + 1];
		if (marker == 0xDA || marker == 0xD9) { // SOS (Start of Scan) or EOI
			clean.append(jpegData.mid(pos));
			break;
		}

		const int segLen = (static_cast<int>(data[pos + 2]) << 8) | static_cast<int>(data[pos + 3]);
		if (pos + 2 + segLen > len || segLen < 2) {
			return jpegData; // Malformed segment, fallback
		}

		// APP1 (0xE1) contains EXIF / GPS metadata
		// APP2 (0xE2) can contain FlashPix / ICC
		if (marker == 0xE1) {
			// Strip EXIF / GPS segment
			pos += 2 + segLen;
			continue;
		}

		clean.append(jpegData.mid(pos, 2 + segLen));
		pos += 2 + segLen;
	}

	return clean.isEmpty() ? jpegData : clean;
}

void SecurityManager::triggerPanicLock() {
	// 1. Clear clipboard to remove sensitive tokens/passwords
	if (auto clip = QGuiApplication::clipboard()) {
		clip->clear();
	}
	// 2. Lock application passcode if configured
	if (Core::IsAppLaunched()) {
		Core::App().lockByPasscode();
	}
	Ui::Toast::Show(QString::fromUtf8("Экстренная блокировка Rekogram активирована"));
}

void SecurityManager::resetToDefaults() {
	_antiPhishingEnabled = true;
	_strictP2PRelay = true;
	_stripMediaMetadata = true;
	_autoClearSearchOnLock = true;
	_blurSensitiveMedia = false;
	save();
}

} // namespace Security
} // namespace Rekogram
