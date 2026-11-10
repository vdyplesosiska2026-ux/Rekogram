/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <rpl/variable.h>

namespace Rekogram {
namespace Themes {

enum class ChatDensity {
	Compact = 0,  // 48px row
	Standard = 1, // 64px row (default)
	Relaxed = 2,  // 76px row
};

enum class TimestampFormat {
	Standard24 = 0, // "14:25"
	Standard12 = 1, // "2:25 PM"
	WithSeconds = 2,// "14:25:30"
	Short = 3,      // "14:25"
};

struct AccentPreset {
	QString id;
	QString name;
	QString hexColor;
};

class CustomizerManager final {
public:
	static CustomizerManager &Instance();

	// Feature 7: Custom Chat Bubble Corner Radius
	[[nodiscard]] int bubbleRadius() const { return _bubbleRadius; }
	void setBubbleRadius(int radius);

	// Feature 8: Custom Monospace Code Font
	[[nodiscard]] QString monospaceFont() const { return _monospaceFont; }
	void setMonospaceFont(const QString &fontFamily);
	[[nodiscard]] QStringList availableMonospaceFonts() const;

	// Feature 9: Accent Color Presets
	[[nodiscard]] QString accentColor() const { return _accentColor; }
	void setAccentColor(const QString &hex);
	[[nodiscard]] std::vector<AccentPreset> accentPresets() const;

	// Feature 10: Hide "All Chats" Folder Tab
	[[nodiscard]] bool hideAllChatsTab() const { return _hideAllChatsTab; }
	void setHideAllChatsTab(bool hide);

	// Feature 11: Custom Chat List Density
	[[nodiscard]] ChatDensity chatDensity() const { return _chatDensity; }
	void setChatDensity(ChatDensity density);
	[[nodiscard]] int chatRowHeight() const;

	// Feature 12: Message Text Size Delta
	[[nodiscard]] int messageTextDelta() const { return _messageTextDelta; }
	void setMessageTextDelta(int delta);

	// Feature 13: Hide Stories Bar / Tray
	[[nodiscard]] bool hideStoriesBar() const { return _hideStoriesBar; }
	void setHideStoriesBar(bool hide);

	// Feature 14: Custom Timestamp Format
	[[nodiscard]] TimestampFormat timestampFormat() const { return _timestampFormat; }
	void setTimestampFormat(TimestampFormat format);
	[[nodiscard]] QString formatTime(const QTime &time) const;

	// Feature 31: Rounded Buttons & UI Geometry
	[[nodiscard]] bool roundedButtons() const { return _roundedButtons; }
	void setRoundedButtons(bool rounded);
	[[nodiscard]] int buttonRadius() const { return _buttonRadius; }
	void setButtonRadius(int radius);

	// Feature 32: Custom UI Font & Google Sans CDN
	[[nodiscard]] QString uiFont() const { return _uiFont; }
	void setUiFont(const QString &fontFamily);
	[[nodiscard]] QStringList availableUiFonts() const;
	void applyFont(const QString &fontFamily);
	void loadGoogleSansCdn();

	void resetToDefaults();

private:
	CustomizerManager();
	void load();
	void save();

	int _bubbleRadius = 12;
	QString _monospaceFont = u"Consolas"_q;
	QString _accentColor = u"#0071E3"_q;
	bool _hideAllChatsTab = false;
	ChatDensity _chatDensity = ChatDensity::Standard;
	int _messageTextDelta = 0;
	bool _hideStoriesBar = false;
	TimestampFormat _timestampFormat = TimestampFormat::Standard24;
	bool _roundedButtons = true;
	int _buttonRadius = 14;
	QString _uiFont = u"Google Sans"_q;
};

} // namespace Themes
} // namespace Rekogram
