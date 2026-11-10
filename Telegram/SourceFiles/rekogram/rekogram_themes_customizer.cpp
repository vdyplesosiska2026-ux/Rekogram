/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_themes_customizer.h"

#include "core/application.h"
#include "core/core_settings.h"
#include <QtCore/QTime>
#include <QtGui/QFontDatabase>

namespace Rekogram {
namespace Themes {
namespace {

constexpr auto kBubbleRadiusPref = "reko_thm_bubble_radius";
constexpr auto kMonoFontPref = "reko_thm_mono_font";
constexpr auto kAccentColorPref = "reko_thm_accent_color";
constexpr auto kHideAllChatsTabPref = "reko_thm_hide_all_chats_tab";
constexpr auto kChatDensityPref = "reko_thm_chat_density";
constexpr auto kMessageTextDeltaPref = "reko_thm_msg_text_delta";
constexpr auto kHideStoriesBarPref = "reko_thm_hide_stories_bar";
constexpr auto kTimestampFormatPref = "reko_thm_timestamp_format";

} // namespace

CustomizerManager &CustomizerManager::Instance() {
	static CustomizerManager instance;
	return instance;
}

CustomizerManager::CustomizerManager() {
	load();
}

void CustomizerManager::load() {
	auto &core = Core::App().settings();
	_bubbleRadius = core.readPref<QByteArray>(kBubbleRadiusPref, QByteArray::number(12)).toInt();
	_monospaceFont = QString::fromUtf8(core.readPref<QByteArray>(kMonoFontPref, "Consolas"));
	_accentColor = QString::fromUtf8(core.readPref<QByteArray>(kAccentColorPref, "#0071E3"));
	_hideAllChatsTab = core.readPref<bool>(kHideAllChatsTabPref, false);
	_chatDensity = static_cast<ChatDensity>(core.readPref<QByteArray>(kChatDensityPref, "1").toInt());
	_messageTextDelta = core.readPref<QByteArray>(kMessageTextDeltaPref, "0").toInt();
	_hideStoriesBar = core.readPref<bool>(kHideStoriesBarPref, false);
	_timestampFormat = static_cast<TimestampFormat>(core.readPref<QByteArray>(kTimestampFormatPref, "0").toInt());
}

void CustomizerManager::save() {
	auto &core = Core::App().settings();
	core.writePref<QByteArray>(kBubbleRadiusPref, QByteArray::number(_bubbleRadius));
	core.writePref<QByteArray>(kMonoFontPref, _monospaceFont.toUtf8());
	core.writePref<QByteArray>(kAccentColorPref, _accentColor.toUtf8());
	core.writePref<bool>(kHideAllChatsTabPref, _hideAllChatsTab);
	core.writePref<QByteArray>(kChatDensityPref, QByteArray::number(static_cast<int>(_chatDensity)));
	core.writePref<QByteArray>(kMessageTextDeltaPref, QByteArray::number(_messageTextDelta));
	core.writePref<bool>(kHideStoriesBarPref, _hideStoriesBar);
	core.writePref<QByteArray>(kTimestampFormatPref, QByteArray::number(static_cast<int>(_timestampFormat)));
}

void CustomizerManager::setBubbleRadius(int radius) {
	const auto clamped = std::clamp(radius, 0, 24);
	if (_bubbleRadius != clamped) {
		_bubbleRadius = clamped;
		save();
	}
}

void CustomizerManager::setMonospaceFont(const QString &fontFamily) {
	if (_monospaceFont != fontFamily && !fontFamily.isEmpty()) {
		_monospaceFont = fontFamily;
		save();
	}
}

QStringList CustomizerManager::availableMonospaceFonts() const {
	return {
		u"Consolas"_q,
		u"Cascadia Code"_q,
		u"JetBrains Mono"_q,
		u"Fira Code"_q,
		u"Courier New"_q,
	};
}

void CustomizerManager::setAccentColor(const QString &hex) {
	if (_accentColor != hex && hex.startsWith('#')) {
		_accentColor = hex;
		save();
	}
}

std::vector<AccentPreset> CustomizerManager::accentPresets() const {
	return {
		{ u"sapphire"_q, QString::fromUtf8("Sapphire Azure"), u"#0071E3"_q },
		{ u"emerald"_q, QString::fromUtf8("Emerald Green"), u"#34C759"_q },
		{ u"amethyst"_q, QString::fromUtf8("Amethyst Purple"), u"#AF52DE"_q },
		{ u"sunset"_q, QString::fromUtf8("Sunset Amber"), u"#FF9500"_q },
		{ u"titanium"_q, QString::fromUtf8("Titanium Graphite"), u"#8E8E93"_q },
		{ u"crimson"_q, QString::fromUtf8("Crimson Red"), u"#FF3B30"_q },
	};
}

void CustomizerManager::setHideAllChatsTab(bool hide) {
	if (_hideAllChatsTab != hide) {
		_hideAllChatsTab = hide;
		save();
	}
}

void CustomizerManager::setChatDensity(ChatDensity density) {
	if (_chatDensity != density) {
		_chatDensity = density;
		save();
	}
}

int CustomizerManager::chatRowHeight() const {
	switch (_chatDensity) {
	case ChatDensity::Compact: return 48;
	case ChatDensity::Relaxed: return 76;
	case ChatDensity::Standard:
	default: return 64;
	}
}

void CustomizerManager::setMessageTextDelta(int delta) {
	const auto clamped = std::clamp(delta, -2, 6);
	if (_messageTextDelta != clamped) {
		_messageTextDelta = clamped;
		save();
	}
}

void CustomizerManager::setHideStoriesBar(bool hide) {
	if (_hideStoriesBar != hide) {
		_hideStoriesBar = hide;
		save();
	}
}

void CustomizerManager::setTimestampFormat(TimestampFormat format) {
	if (_timestampFormat != format) {
		_timestampFormat = format;
		save();
	}
}

QString CustomizerManager::formatTime(const QTime &time) const {
	switch (_timestampFormat) {
	case TimestampFormat::Standard12:
		return time.toString(u"h:mm AP"_q);
	case TimestampFormat::WithSeconds:
		return time.toString(u"HH:mm:ss"_q);
	case TimestampFormat::Short:
	case TimestampFormat::Standard24:
	default:
		return time.toString(u"HH:mm"_q);
	}
}

void CustomizerManager::resetToDefaults() {
	_bubbleRadius = 12;
	_monospaceFont = u"Consolas"_q;
	_accentColor = u"#0071E3"_q;
	_hideAllChatsTab = false;
	_chatDensity = ChatDensity::Standard;
	_messageTextDelta = 0;
	_hideStoriesBar = false;
	_timestampFormat = TimestampFormat::Standard24;
	save();
}

} // namespace Themes
} // namespace Rekogram
