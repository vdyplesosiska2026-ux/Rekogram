/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>

namespace Window {
class SessionController;
} // namespace Window

namespace Rekogram {
namespace BookmarksUI {

void Show(not_null<Window::SessionController*> controller);

} // namespace BookmarksUI
} // namespace Rekogram
