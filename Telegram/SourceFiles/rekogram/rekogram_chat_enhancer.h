/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Rekogram {
namespace ChatEnhancer {

enum class DoubleClickAction {
	Reply = 0,
	ReactHeart = 1,
	CopyText = 2,
	Edit = 3,
};

class EnhancerManager final {
public:
	static EnhancerManager &Instance();

	// Feature 15: One-Click Copy Link & Text
	[[nodiscard]] QString formatMessageForCopy(const QString &text, const QString &link) const;

	// Feature 16: Double-Click Action on Message
	[[nodiscard]] DoubleClickAction doubleClickAction() const { return _doubleClickAction; }
	void setDoubleClickAction(DoubleClickAction action);
	[[nodiscard]] QString doubleClickActionName() const;

	// Feature 17: Show Character & Byte Counter in Compose
	[[nodiscard]] bool showCharByteCounter() const { return _showCharByteCounter; }
	void setShowCharByteCounter(bool show);
	[[nodiscard]] QString formatLengthInfo(const QString &text) const;

	// Feature 18: Destructive Action Confirmations Guard
	[[nodiscard]] bool confirmDestructiveActions() const { return _confirmDestructiveActions; }
	void setConfirmDestructiveActions(bool confirm);

	// Feature 19: Auto-Mute Microphone on Group Call Join
	[[nodiscard]] bool autoMuteOnJoin() const { return _autoMuteOnJoin; }
	void setAutoMuteOnJoin(bool mute);

	// Feature 20: Fast Avatar / Mention Shortcuts
	[[nodiscard]] bool fastMentionShortcuts() const { return _fastMentionShortcuts; }
	void setFastMentionShortcuts(bool fast);

	// Feature 21: Forward without Quoting (Clean Copy)
	[[nodiscard]] bool cleanForwardWithoutQuoting() const { return _cleanForwardWithoutQuoting; }
	void setCleanForwardWithoutQuoting(bool clean);

	// Feature 22: Disable Sticker & Emoji Auto-Play
	[[nodiscard]] bool disableStickerAutoPlay() const { return _disableStickerAutoPlay; }
	void setDisableStickerAutoPlay(bool disable);

	void resetToDefaults();

private:
	EnhancerManager();
	void load();
	void save();

	DoubleClickAction _doubleClickAction = DoubleClickAction::Reply;
	bool _showCharByteCounter = true;
	bool _confirmDestructiveActions = true;
	bool _autoMuteOnJoin = true;
	bool _fastMentionShortcuts = true;
	bool _cleanForwardWithoutQuoting = false;
	bool _disableStickerAutoPlay = false;
};

} // namespace ChatEnhancer
} // namespace Rekogram
