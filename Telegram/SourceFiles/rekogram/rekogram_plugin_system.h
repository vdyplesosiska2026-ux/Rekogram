/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <vector>

namespace Rekogram {

struct PluginManifest {
	QString id;
	QString name;
	QString version;
	QString author;
	QString description;
	bool enabled = true;
};

class PluginManager final {
public:
	static PluginManager &Instance();

	[[nodiscard]] std::vector<PluginManifest> plugins() const;
	void setPluginEnabled(const QString &id, bool enabled);
	[[nodiscard]] bool isPluginEnabled(const QString &id) const;

private:
	PluginManager();
	void load();
	void save();

	std::vector<PluginManifest> _plugins;
};

} // namespace Rekogram
