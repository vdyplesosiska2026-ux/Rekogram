/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_chat_enhancer.h"

#include "core/application.h"
#include "core/core_settings.h"

namespace Rekogram {
namespace ChatEnhancer {
namespace {

constexpr auto kDoubleClickActionPref = "reko_enh_double_click";
constexpr auto kShowCharCounterPref = "reko_enh_show_char_counter";
constexpr auto kConfirmDestructivePref = "reko_enh_confirm_destructive";
constexpr auto kAutoMuteJoinPref = "reko_enh_auto_mute_join";
constexpr auto kFastMentionPref = "reko_enh_fast_mention";
constexpr auto kCleanForwardPref = "reko_enh_clean_forward";
constexpr auto kDisableStickerAnimPref = "reko_enh_disable_sticker_anim";

} // namespace

EnhancerManager &EnhancerManager::Instance() {
	static EnhancerManager instance;
	return instance;
}

EnhancerManager::EnhancerManager() {
	load();
}

void EnhancerManager::load() {
	auto &core = Core::App().settings();
	_doubleClickAction = static_cast<DoubleClickAction>(core.readPref<QByteArray>(kDoubleClickActionPref, "0").toInt());
	_showCharByteCounter = core.readPref<bool>(kShowCharCounterPref, true);
	_confirmDestructiveActions = core.readPref<bool>(kConfirmDestructivePref, true);
	_autoMuteOnJoin = core.readPref<bool>(kAutoMuteJoinPref, true);
	_fastMentionShortcuts = core.readPref<bool>(kFastMentionPref, true);
	_cleanForwardWithoutQuoting = core.readPref<bool>(kCleanForwardPref, false);
	_disableStickerAutoPlay = core.readPref<bool>(kDisableStickerAnimPref, false);
}

void EnhancerManager::save() {
	auto &core = Core::App().settings();
	core.writePref<QByteArray>(kDoubleClickActionPref, QByteArray::number(static_cast<int>(_doubleClickAction)));
	core.writePref<bool>(kShowCharCounterPref, _showCharByteCounter);
	core.writePref<bool>(kConfirmDestructivePref, _confirmDestructiveActions);
	core.writePref<bool>(kAutoMuteJoinPref, _autoMuteOnJoin);
	core.writePref<bool>(kFastMentionPref, _fastMentionShortcuts);
	core.writePref<bool>(kCleanForwardPref, _cleanForwardWithoutQuoting);
	core.writePref<bool>(kDisableStickerAnimPref, _disableStickerAutoPlay);
}

QString EnhancerManager::formatMessageForCopy(const QString &text, const QString &link) const {
	if (link.isEmpty()) return text;
	if (text.isEmpty()) return link;
	return text + u"\n\n🔗 "_q + link;
}

void EnhancerManager::setDoubleClickAction(DoubleClickAction action) {
	if (_doubleClickAction != action) {
		_doubleClickAction = action;
		save();
	}
}

QString EnhancerManager::doubleClickActionName() const {
	switch (_doubleClickAction) {
	case DoubleClickAction::ReactHeart:
		return QString::fromUtf8("Реакция ❤️");
	case DoubleClickAction::CopyText:
		return QString::fromUtf8("Копировать текст");
	case DoubleClickAction::Edit:
		return QString::fromUtf8("Редактировать");
	case DoubleClickAction::Reply:
	default:
		return QString::fromUtf8("Ответить на сообщение");
	}
}

void EnhancerManager::setShowCharByteCounter(bool show) {
	if (_showCharByteCounter != show) {
		_showCharByteCounter = show;
		save();
	}
}

QString EnhancerManager::formatLengthInfo(const QString &text) const {
	const auto chars = text.size();
	const auto bytes = text.toUtf8().size();
	return QString::number(chars) + u" симв. / "_q + QString::number(bytes) + u" байт"_q;
}

void EnhancerManager::setConfirmDestructiveActions(bool confirm) {
	if (_confirmDestructiveActions != confirm) {
		_confirmDestructiveActions = confirm;
		save();
	}
}

void EnhancerManager::setAutoMuteOnJoin(bool mute) {
	if (_autoMuteOnJoin != mute) {
		_autoMuteOnJoin = mute;
		save();
	}
}

void EnhancerManager::setFastMentionShortcuts(bool fast) {
	if (_fastMentionShortcuts != fast) {
		_fastMentionShortcuts = fast;
		save();
	}
}

void EnhancerManager::setCleanForwardWithoutQuoting(bool clean) {
	if (_cleanForwardWithoutQuoting != clean) {
		_cleanForwardWithoutQuoting = clean;
		save();
	}
}

void EnhancerManager::setDisableStickerAutoPlay(bool disable) {
	if (_disableStickerAutoPlay != disable) {
		_disableStickerAutoPlay = disable;
		save();
	}
}

void EnhancerManager::resetToDefaults() {
	_doubleClickAction = DoubleClickAction::Reply;
	_showCharByteCounter = true;
	_confirmDestructiveActions = true;
	_autoMuteOnJoin = true;
	_fastMentionShortcuts = true;
	_cleanForwardWithoutQuoting = false;
	_disableStickerAutoPlay = false;
	save();
}

} // namespace ChatEnhancer
} // namespace Rekogram
