#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h"
#include "storage.h"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

static String pendingUID     = "";
static bool   waitingForName = false;

//  Wi-Fi підключення
bool wifiConnect() {
    WiFi.disconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("[WiFi] Connecting");
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - t > WIFI_TIMEOUT_MS) {
            Serial.println("\n[WiFi] Timeout!");
            return false;
        }
        delay(300);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] OK  IP: " + WiFi.localIP().toString());
    secured_client.setInsecure();
    secured_client.setTimeout(15000);
    return true;
}

bool wifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

//  Надсилання з повтором
static bool tgSend(const String& msg, const String& parse = "Markdown") {
    if (!wifiConnected()) { Serial.println("[TG] No WiFi"); return false; }
    for (int i = 1; i <= 2; i++) {
        Serial.printf("[TG] Send attempt %d...\n", i);
        if (bot.sendMessage(ADMIN_CHAT_ID, msg, parse)) {
            Serial.println("[TG] OK");
            return true;
        }
        delay(600);
    }
    Serial.println("[TG] Failed");
    return false;
}

static bool tgSendKeyboard(const String& msg, const String& kb) {
    if (!wifiConnected()) return false;
    for (int i = 1; i <= 2; i++) {
        Serial.printf("[TG] Send keyboard attempt %d...\n", i);
        if (bot.sendMessageWithInlineKeyboard(ADMIN_CHAT_ID, msg, "Markdown", kb)) {
            Serial.println("[TG] OK");
            return true;
        }
        delay(600);
    }
    Serial.println("[TG] Keyboard failed");
    return false;
}

//  Сповіщення: доступ дозволено
void tgNotifyGranted(const String& name, const String& uid) {
    String msg = "✅ *Доступ надано*\n";
    msg += "👤 " + name + "\n";
    msg += "🔑 UID: `" + uid + "`";
    tgSend(msg);
}

//  Сповіщення: невідома картка
void tgNotifyUnknown(const String& uid) {
    String msg = "⚠️ *Невідома картка*\n";
    msg += "🔑 UID: `" + uid + "`\n\n";
    msg += "Що зробити?\n\n";
    
    // Командний інтерфейс замість кнопок
    msg += "➡️ Додати: `/add " + uid + "`\n";
    msg += "➡️ Відхилити: `/deny " + uid + "`";

    tgSend(msg);
}

//  Обробка повідомлень
void tgHandleMessages(int n) {
    for (int i = 0; i < n; i++) {
        telegramMessage& msg = bot.messages[i];

        if (msg.chat_id != String(ADMIN_CHAT_ID)) {
            bot.sendMessage(msg.chat_id, "⛔ Access denied.", "");
            continue;
        }

        // Обробка тільки звичайних повідомлень
        if (msg.type == "message") {
            String text = msg.text;
            text.trim();

            // ✅ СПОЧАТКУ - перевірка очікування імені для додавання картки
            if (waitingForName && pendingUID.length() > 0) {
                String name = text;
                name.trim();
                
                if (name.length() == 0) {
                    bot.sendMessage(ADMIN_CHAT_ID, "⚠️ Ім'я порожнє, спробуй ще раз.", "");
                    continue;
                }
                
                if (addCard(pendingUID, name)) {
                    bot.sendMessage(ADMIN_CHAT_ID,
                        "✅ Додано!\n👤 *" + name + "*\n🔑 `" + pendingUID + "`",
                        "Markdown");
                } else {
                    bot.sendMessage(ADMIN_CHAT_ID, "⚠️ Вже існує або помилка SD.", "");
                }
                
                pendingUID     = "";
                waitingForName = false;
                continue;
            }

            // ✅ ПОТІМ - перевірка команд
            if (text.startsWith("/add ")) {
                pendingUID = text.substring(5);
                pendingUID.trim();
                waitingForName = true;
                bot.sendMessage(ADMIN_CHAT_ID,
                    "📝 Введіть ім'я для `" + pendingUID + "`",
                    "Markdown"
                );
            }
            // ВІДХИЛИТИ КАРТКУ
            else if (text.startsWith("/deny ")) {
                String uid = text.substring(6);
                uid.trim();
                bot.sendMessage(ADMIN_CHAT_ID,
                    "❌ Картка `" + uid + "` відхилена.",
                    "Markdown"
                );
            }
            // ДОПОМОГА
            else if (text == "/start" || text == "/help") {
                String h = "🔐 *Access Control Bot*\n\n";
                h += "/list — список карток\n";
                h += "/log — останні 10 подій\n";
                h += "/delete UID — видалити картку\n";
                h += "/status — стан системи";
                bot.sendMessage(ADMIN_CHAT_ID, h, "Markdown");
            }
            // СПИСОК КАРТОК
            else if (text == "/list") {
                bot.sendMessage(ADMIN_CHAT_ID, listCards(), "Markdown");
            }
            // ОСТАННІ ЛОГИ
            else if (text == "/log") {
                bot.sendMessage(ADMIN_CHAT_ID, getLastLogs(10), "Markdown");
            }
            // ВИДАЛИТИ КАРТКУ
            else if (text.startsWith("/delete ")) {
                String uid = text.substring(8);
                uid.trim();
                uid.toUpperCase();
                
                if (removeCard(uid)) {
                    bot.sendMessage(ADMIN_CHAT_ID,
                        "🗑 Видалено: `" + uid + "`", "Markdown");
                } else {
                    bot.sendMessage(ADMIN_CHAT_ID,
                        "❌ Не знайдено: `" + uid + "`", "Markdown");
                }
            }
            // СТАТУС СИСТЕМИ
            else if (text == "/status") {
                extern bool sdAvailable;
                String s = "📊 *Статус*\n";
                s += "📶 IP: " + WiFi.localIP().toString() + "\n";
                s += "💾 SD: " + String(sdAvailable ? "OK ✅" : "ERROR ❌") + "\n";
                s += "⏱ Uptime: " + String(millis() / 60000) + " хв";
                bot.sendMessage(ADMIN_CHAT_ID, s, "Markdown");
            }
            // НЕВІДОМА КОМАНДА
            else {
                bot.sendMessage(ADMIN_CHAT_ID, "❓ Невідома команда. /help", "");
            }
        }
    }
}

//  Polling
static unsigned long lastPoll = 0;

void tgPoll() {
    if (!wifiConnected()) return;
    if (millis() - lastPoll < TELEGRAM_POLL_MS) return;
    lastPoll = millis();
    int n = bot.getUpdates(bot.last_message_received + 1);
    if (n > 0) {
        Serial.printf("[TG] %d new message(s)\n", n);
        tgHandleMessages(n);
    }
}
