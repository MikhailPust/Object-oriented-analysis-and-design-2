import os
import smtplib
import time
from abc import ABC, abstractmethod
from datetime import datetime
from email.message import EmailMessage

import requests

GMAIL_ADDRESS = "Введите аодрес электронной почты отправителя"
GMAIL_PASSWORD = "Пароль от приложения 16 символов"
TELEGRAM_TOKEN = "Введите токен телеграм"

# Абсолютный путь к папке проекта
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_FILE = os.path.join(BASE_DIR, "notifications.log")

class NotificationSender(ABC):
    """
    Шаблонный метод:
    1. format_message()
    2. validate_recipient()
    3. send()
    4. log()
    """

    def send_notification(self, recipient: str, subject: str, message: str) -> dict:
        result = {
            "channel": self.channel_name(),
            "recipient": recipient,
            "timestamp": datetime.now().strftime("%H:%M:%S"),
            "success": False,
            "error": None,
        }

        try:
            formatted = self.format_message(subject, message)
            self.validate_recipient(recipient)
            self.send(recipient, formatted["subject"], formatted["body"])
            result["success"] = True

        except Exception as exc:
            result["error"] = str(exc)

        finally:
            self.log(result)

        return result

    def format_message(self, subject: str, body: str) -> dict:
        return {
            "subject": subject.strip(),
            "body": f"{body.strip()}\n\n— Отправлено через Notification Center",
        }

    def log(self, result: dict) -> None:
        status = "OK" if result["success"] else f"ОШИБКА: {result['error']}"
        log_line = (
            f"[{result['timestamp']}] "
            f"{result['channel']:10} -> {result['recipient']:30} | {status}\n"
        )

        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(log_line)

    @abstractmethod
    def send(self, recipient: str, subject: str, body: str) -> None:
        pass

    @abstractmethod
    def validate_recipient(self, recipient: str) -> None:
        pass

    @abstractmethod
    def channel_name(self) -> str:
        pass


class EmailSender(NotificationSender):
    """Отправляет письма через Gmail SMTP."""

    def __init__(self, gmail_address: str, app_password: str):
        self.gmail_address = gmail_address
        self.app_password = app_password

    def channel_name(self) -> str:
        return "Email"

    def validate_recipient(self, recipient: str) -> None:
        if "@" not in recipient:
            raise ValueError("Email должен содержать символ '@'")

        domain = recipient.split("@")[-1]
        if "." not in domain:
            raise ValueError("Некорректный email-адрес")

    def send(self, recipient: str, subject: str, body: str) -> None:
        if not self.gmail_address or "@" not in self.gmail_address:
            raise ValueError("Не задан корректный адрес Gmail отправителя")

        if not self.app_password or len(self.app_password.replace(" ", "")) < 16:
            raise ValueError("Неверный пароль приложения Gmail")

        msg = EmailMessage()
        msg["From"] = self.gmail_address
        msg["To"] = recipient
        msg["Subject"] = subject
        msg.set_content(body)

        try:
            with smtplib.SMTP_SSL("smtp.gmail.com", 465, timeout=15) as server:
                server.login(self.gmail_address, self.app_password.replace(" ", ""))
                server.send_message(msg)
        except smtplib.SMTPAuthenticationError:
            raise ValueError(
                "Ошибка авторизации Gmail. Проверь почту и пароль приложения."
            )
        except smtplib.SMTPException as e:
            raise ConnectionError(f"SMTP ошибка: {e}")
        except OSError as e:
            raise ConnectionError(f"Сетевая ошибка Email: {e}")


class TelegramSender(NotificationSender):

    _API_URL = "https://api.telegram.org/bot{token}/sendMessage"

    def __init__(self, bot_token: str):
        self.bot_token = bot_token

    def channel_name(self) -> str:
        return "Telegram"

    def validate_recipient(self, recipient: str) -> None:
        try:
            int(recipient)
        except ValueError:
            raise ValueError("Telegram chat_id должен быть числом")

    def send(self, recipient: str, subject: str, body: str) -> None:
        if not self.bot_token:
            raise ValueError("Не задан Telegram token")

        url = self._API_URL.format(token=self.bot_token)
        text = f"{subject}\n\n{body}"

        try:
            response = requests.post(
                url,
                json={
                    "chat_id": recipient,
                    "text": text
                },
                timeout=10
            )
        except requests.RequestException as e:
            raise ConnectionError(f"Ошибка сети Telegram: {e}")

        try:
            data = response.json()
        except Exception:
            raise ConnectionError("Telegram API вернул некорректный ответ")

        if not response.ok or not data.get("ok", False):
            desc = data.get("description", "Неизвестная ошибка Telegram API")
            raise ConnectionError(desc)

class SmsSender(NotificationSender):

    def channel_name(self) -> str:
        return "SMS"

    def validate_recipient(self, recipient: str) -> None:
        cleaned = recipient.replace(" ", "").replace("-", "")
        if not cleaned.startswith("+"):
            raise ValueError("Номер должен начинаться с '+'")
        if not cleaned[1:].isdigit():
            raise ValueError("После '+' должны идти только цифры")
        if len(cleaned) < 11:
            raise ValueError("Слишком короткий номер телефона")

    def send(self, recipient: str, subject: str, body: str) -> None:
        time.sleep(0.5)
        print(f"[SMS STUB] {recipient} | {subject} | {body}")

def create_senders() -> dict:
    return {
        "Email": EmailSender(GMAIL_ADDRESS, GMAIL_PASSWORD),
        "Telegram": TelegramSender(TELEGRAM_TOKEN),
        "SMS": SmsSender(),
    }
