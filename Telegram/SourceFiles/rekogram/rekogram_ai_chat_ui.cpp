/*
This file is part of Rekogram Desktop,
an enhanced desktop application based on the Telegram source base.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "rekogram/rekogram_ai_chat_ui.h"

#include "rekogram/rekogram_ai_chat.h"
#include "window/window_session_controller.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QDateTime>

namespace Rekogram {
namespace AIChatUI {
namespace {

class AIChatDialog final : public QDialog {
public:
	AIChatDialog(not_null<Window::SessionController*> controller)
	: QDialog(controller->widget()) {
		setWindowTitle(QString::fromUtf8("Rekogram AI Ассистент"));
		setModal(false);
		resize(520, 560);

		auto *layout = new QVBoxLayout(this);
		layout->setContentsMargins(12, 12, 12, 12);
		layout->setSpacing(8);

		_statusLabel = new QLabel(this);
		_statusLabel->setWordWrap(true);
		layout->addWidget(_statusLabel);

		_history = new QTextBrowser(this);
		_history->setOpenExternalLinks(false);
		layout->addWidget(_history, 1);

		auto *inputRow = new QHBoxLayout;
		_input = new QLineEdit(this);
		_input->setPlaceholderText(QString::fromUtf8("Введите сообщение..."));
		_input->setEnabled(AIChatManager::Instance().isConfigured());
		inputRow->addWidget(_input, 1);

		_sendBtn = new QPushButton(QString::fromUtf8("Отправить"), this);
		_sendBtn->setEnabled(AIChatManager::Instance().isConfigured());
		inputRow->addWidget(_sendBtn);
		layout->addLayout(inputRow);

		connect(_sendBtn, &QPushButton::clicked, this, &AIChatDialog::sendMessage);
		connect(_input, &QLineEdit::returnPressed, this, &AIChatDialog::sendMessage);

		refreshHistory();
		updateStatus();
	}

private:
	void updateStatus() {
		const auto &ai = AIChatManager::Instance();
		if (ai.isConfigured()) {
			_statusLabel->setText(QString::fromUtf8("Провайдер: %1").arg(ai.providerName()));
		} else {
			_statusLabel->setText(QString::fromUtf8(
				"AI не настроен. Укажите API-ключ в настройках Rekogram → AI."));
		}
	}

	void refreshHistory() {
		_history->clear();
		const auto messages = AIChatManager::Instance().messages();
		for (const auto &msg : messages) {
			const auto prefix = msg.isOutgoing
				? QString::fromUtf8("<b>Вы:</b> ")
				: QString::fromUtf8("<b>AI:</b> ");
			_history->append(prefix + msg.text.toHtmlEscaped());
		}
		_history->verticalScrollBar()->setValue(
			_history->verticalScrollBar()->maximum());
	}

	void sendMessage() {
		const auto text = _input->text().trimmed();
		if (text.isEmpty()) return;
		if (!AIChatManager::Instance().isConfigured()) {
			return;
		}
		_input->clear();
		AIChatManager::Instance().sendMessage(text);
		refreshHistory();
	}

	QLabel *_statusLabel = nullptr;
	QTextBrowser *_history = nullptr;
	QLineEdit *_input = nullptr;
	QPushButton *_sendBtn = nullptr;
};

} // namespace

void Show(not_null<Window::SessionController*> controller) {
	auto *dialog = new AIChatDialog(controller);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

} // namespace AIChatUI
} // namespace Rekogram
