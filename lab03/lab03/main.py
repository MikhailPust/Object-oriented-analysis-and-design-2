
import threading
import customtkinter as ctk

from backend import create_senders   # ← единственный импорт из нашего кода

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")


class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("🔔 Notification Center")
        self.geometry("620x700")
        self.resizable(False, False)

        # Все отправители берём из backend (там живёт паттерн)
        self.senders = create_senders()

        self._build_ui()

    def _build_ui(self):
        # Заголовок
        header = ctk.CTkFrame(self, fg_color="#1a1a2e", corner_radius=0)
        header.pack(fill="x")
        ctk.CTkLabel(
            header, text="🔔 Notification Center",
            font=ctk.CTkFont(size=22, weight="bold"),
            text_color="#4fc3f7",
        ).pack(pady=16)

        main = ctk.CTkFrame(self, fg_color="transparent")
        main.pack(fill="both", expand=True, padx=24, pady=12)

        # Выбор канала
        ctk.CTkLabel(main, text="Канал отправки",
                     font=ctk.CTkFont(size=13, weight="bold")).pack(anchor="w")

        self.channel_var = ctk.StringVar(value="Email")
        ch_frame = ctk.CTkFrame(main, fg_color="transparent")
        ch_frame.pack(fill="x", pady=(4, 14))

        for label, value in [("📧  Email", "Email"),
                              ("✈️  Telegram", "Telegram"),
                              ("📱  SMS", "SMS")]:
            ctk.CTkRadioButton(
                ch_frame, text=label, value=value,
                variable=self.channel_var,
                command=self._on_channel_change,
                font=ctk.CTkFont(size=13),
            ).pack(side="left", padx=12)

        # Подсказка под выбором канала
        self.hint_label = ctk.CTkLabel(main, text="",
                                       text_color="#888",
                                       font=ctk.CTkFont(size=11))
        self.hint_label.pack(anchor="w")

        # Получатель
        ctk.CTkLabel(main, text="Получатель",
                     font=ctk.CTkFont(size=13, weight="bold")).pack(anchor="w", pady=(8, 0))
        self.recipient_entry = ctk.CTkEntry(
            main, placeholder_text="example@gmail.com",
            height=40, font=ctk.CTkFont(size=13),
        )
        self.recipient_entry.pack(fill="x", pady=(4, 12))

        # Тема
        ctk.CTkLabel(main, text="Тема",
                     font=ctk.CTkFont(size=13, weight="bold")).pack(anchor="w")
        self.subject_entry = ctk.CTkEntry(
            main, placeholder_text="Введите тему сообщения",
            height=40, font=ctk.CTkFont(size=13),
        )
        self.subject_entry.pack(fill="x", pady=(4, 12))

        # Текст
        ctk.CTkLabel(main, text="Текст сообщения",
                     font=ctk.CTkFont(size=13, weight="bold")).pack(anchor="w")
        self.message_box = ctk.CTkTextbox(main, height=110, font=ctk.CTkFont(size=13))
        self.message_box.pack(fill="x", pady=(4, 16))

        # Кнопка
        self.send_btn = ctk.CTkButton(
            main, text="📤  Отправить",
            height=44, font=ctk.CTkFont(size=14, weight="bold"),
            fg_color="#1565c0", hover_color="#0d47a1",
            command=self._on_send,
        )
        self.send_btn.pack(fill="x")

        # Прогресс
        self.progress = ctk.CTkProgressBar(main, height=6)
        self.progress.pack(fill="x", pady=(8, 0))
        self.progress.set(0)

        # Статус
        self.status_label = ctk.CTkLabel(main, text="",
                                         font=ctk.CTkFont(size=12))
        self.status_label.pack(pady=(6, 0))

        # Лог
        ctk.CTkLabel(main, text="📋  История отправок",
                     font=ctk.CTkFont(size=13, weight="bold")).pack(anchor="w", pady=(14, 0))
        self.log_box = ctk.CTkTextbox(
            main, height=130,
            font=ctk.CTkFont(family="Courier", size=12),
            state="disabled", fg_color="#0d1117",
        )
        self.log_box.pack(fill="x", pady=(4, 0))

        self._on_channel_change()   # инициализируем подсказку

    # ── СМЕНА КАНАЛА ──────────────────────────────────────────
    def _on_channel_change(self):
        hints = {
            "Email":    ("example@gmail.com",  "Email адрес получателя"),
            "Telegram": ("123456789",           "Chat ID  (узнать: @userinfobot)"),
            "SMS":      ("+79991234567",        "Номер телефона с кодом страны"),
        }
        channel = self.channel_var.get()
        placeholder, hint = hints[channel]
        self.recipient_entry.configure(placeholder_text=placeholder)
        self.hint_label.configure(text=f"ℹ️  {hint}")

    # ── НАЖАТИЕ «ОТПРАВИТЬ» ───────────────────────────────────
    def _on_send(self):
        recipient = self.recipient_entry.get().strip()
        subject   = self.subject_entry.get().strip()
        message   = self.message_box.get("1.0", "end").strip()

        if not recipient or not subject or not message:
            self._set_status("⚠️  Заполните все поля", "#ff9800")
            return

        self.send_btn.configure(state="disabled", text="Отправка...")
        self.progress.set(0)
        self._set_status("Отправляем...", "#4fc3f7")

        # Отправка в фоновом потоке — GUI не зависает
        threading.Thread(
            target=self._send_in_thread,
            args=(recipient, subject, message),
            daemon=True,
        ).start()

    def _send_in_thread(self, recipient, subject, message):
        self.progress.start()
        sender = self.senders[self.channel_var.get()]

        # ← ВЫЗОВ ШАБЛОННОГО МЕТОДА — вся логика внутри backend.py
        result = sender.send_notification(recipient, subject, message)

        self.progress.stop()
        self.progress.set(1 if result["success"] else 0)
        self.after(0, self._on_send_complete, result)   # обновляем UI из главного потока

    def _on_send_complete(self, result):
        if result["success"]:
            self._set_status(
                f"✅  Успешно отправлено через {result['channel']}", "#4caf50"
            )
            self._add_log(
                f"✅ {result['timestamp']}  {result['channel']:10} → {result['recipient']}"
            )
        else:
            self._set_status(f"❌  Ошибка: {result['error']}", "#f44336")
            self._add_log(
                f"❌ {result['timestamp']}  {result['channel']:10} → {result['recipient']}"
                f"  |  {result['error']}"
            )
        self.send_btn.configure(state="normal", text="📤  Отправить")

    def _set_status(self, text: str, color: str):
        self.status_label.configure(text=text, text_color=color)

    def _add_log(self, line: str):
        self.log_box.configure(state="normal")
        self.log_box.insert("end", line + "\n")
        self.log_box.see("end")
        self.log_box.configure(state="disabled")


if __name__ == "__main__":
    App().mainloop()
