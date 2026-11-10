/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QByteArray>

namespace Rekogram {
namespace Security {

enum class UrlSafetyResult {
	Safe,
	DangerousScheme,
	SuspiciousHomograph,
	CredentialSpoofing,
	BlockedHost,
};

struct UrlAuditDetails {
	UrlSafetyResult result = UrlSafetyResult::Safe;
	QString warningMessage;
	QString normalizedHost;
	QString scheme;
	bool isDangerous = false;
};

class SecurityManager final {
public:
	static SecurityManager &Instance();

	// Feature 1: Anti-Phishing & Homograph Shield
	[[nodiscard]] UrlAuditDetails auditUrl(const QString &url) const;
	[[nodiscard]] bool isUrlSafe(const QString &url) const;
	[[nodiscard]] bool antiPhishingEnabled() const { return _antiPhishingEnabled; }
	void setAntiPhishingEnabled(bool enabled);

	// Feature 2: Strict Relay for P2P Calls (Prevents IP Leakage)
	[[nodiscard]] bool strictP2PRelay() const { return _strictP2PRelay; }
	void setStrictP2PRelay(bool enabled);

	// Feature 3: EXIF & Camera Metadata Stripping
	[[nodiscard]] bool stripMediaMetadata() const { return _stripMediaMetadata; }
	void setStripMediaMetadata(bool enabled);
	[[nodiscard]] QByteArray stripExifFromJpeg(const QByteArray &jpegData) const;

	// Feature 4: Auto-Clear Search History on Lock
	[[nodiscard]] bool autoClearSearchOnLock() const { return _autoClearSearchOnLock; }
	void setAutoClearSearchOnLock(bool enabled);

	// Feature 5: Panic Mode / Quick Lock
	void triggerPanicLock();

	// Feature 6: Sensitive Media Blur Overlay
	[[nodiscard]] bool blurSensitiveMedia() const { return _blurSensitiveMedia; }
	void setBlurSensitiveMedia(bool enabled);

	void resetToDefaults();

private:
	SecurityManager();
	void load();
	void save();

	bool _antiPhishingEnabled = true;
	bool _strictP2PRelay = true;
	bool _stripMediaMetadata = true;
	bool _autoClearSearchOnLock = true;
	bool _blurSensitiveMedia = false;
};

// Global inline helpers for core Telegram integrations
[[nodiscard]] inline bool IsUrlAllowed(const QString &url) {
	return SecurityManager::Instance().isUrlSafe(url);
}

[[nodiscard]] inline bool ShouldStripMetadata() {
	return SecurityManager::Instance().stripMediaMetadata();
}

[[nodiscard]] inline bool IsStrictP2PRelayActive() {
	return SecurityManager::Instance().strictP2PRelay();
}

} // namespace Security
} // namespace Rekogram
