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

static String pendingUID = "";
static bool   waitingForName = false;

// Підключення до Wi-Fi
bool wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[WiFi] Підключення");
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - t > WIFI_TIMEOUT_MS) {
            Serial.println("\n[WiFi] Таймаут!");
            return false;
        }
        delay(300); Serial.print(".");
    }
    Serial.println("\n[WiFi] OK  IP: " + WiFi.localIP().toString());
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    return true;
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }

void tgNotifyGranted(const String& name, const String& uid) {
    if (!wifiConnected()) {
        Serial.println("[TG] Wi-Fi OFF - не можу відправити");
        return;
    }
    
    Serial.println("[TG] Надсилання повідомлення про надання доступу...");
    String msg = "✅ *Доступ надано*\n";
    msg += "👤 " + name + "\n";
    msg += "🔑 UID: `" + uid + "`";
    
    delay(300);
    if (bot.sendMessage(ADMIN_CHAT_ID, msg, "Markdown")) {
        Serial.println("[TG] ✓ Повідомлення надіслано");
    } else {
        Serial.println("[TG] ✗ Помилка відправки!");
    }
}

void tgNotifyUnknown(const String& uid) {
    if (!wifiConnected()) {
        Serial.println("[TG] Wi-Fi OFF - не можу відправити");
        return;
    }

    Serial.println("[TG] Надсилання повідомлення про невідому карту...");
    String msg = "⚠️ *Невідома картка*\n";
    msg += "🔑 UID: `" + uid + "`\n\n";
    msg += "Що зробити?";

    String keyboard = "{\"inline_keyboard\":[[";
    keyboard += "{\"text\":\"✅ Додати\",\"callback_data\":\"add_" + uid + "\"},";
    keyboard += "{\"text\":\"❌ Відмовити\",\"callback_data\":\"deny_" + uid + "\"}";
    keyboard += "]]}";

    delay(300);
    if (bot.sendMessageWithInlineKeyboard(ADMIN_CHAT_ID, msg, "Markdown", keyboard)) {
        Serial.println("[TG] ✓ Повідомлення надіслано");
    } else {
        Serial.println("[TG] ✗ Помилка відправки!");
    }
}

void tgHandleMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        telegramMessage &msg = bot.messages[i];

        // Перевірка що писав саме адмін
        if (msg.chat_id != String(ADMIN_CHAT_ID)) {
            bot.sendMessage(msg.chat_id, "⛔ Доступ заборонений.", "");
            continue;
        }

        if (msg.type == "callback_query") {
            String data = msg.text;
            bot.answerCallbackQuery(msg.query_id, "");

            if (data.startsWith("add_")) {
                pendingUID = data.substring(4);
                waitingForName = true;
                bot.sendMessage(ADMIN_CHAT_ID,
                    "📝 Введіть ім'я для картки `" + pendingUID + "`:", "Markdown");

            } else if (data.startsWith("deny_")) {
                String uid = data.substring(5);
                bot.sendMessage(ADMIN_CHAT_ID,
                    "❌ Картка `" + uid + "` відхилена.", "Markdown");
            }
            continue;
        }

        // ── Якщо чекаємо ім'я після натискання "Додати" ──
        if (waitingForName && pendingUID.length() > 0) {
            String name = msg.text;
            name.trim();
            if (addCard(pendingUID, name)) {
                bot.sendMessage(ADMIN_CHAT_ID,
                    "✅ Картку додано!\n👤 " + name + "\n `" + pendingUID + "`",
                    "Markdown");
            } else {
                bot.sendMessage(ADMIN_CHAT_ID, "Картка вже існує або помилка SD.", "");
            }
            pendingUID = "";
            waitingForName = false;
            continue;
        }

        String text = msg.text;
        text.trim();

        // ── Команди ──
        if (text == "/start" || text == "/help") {
            String help = "*Access Control Bot*\n\n";
            help += "/list - список карток\n";
            help += "/log - останні події\n";
            help += "/delete + UID - видалити картку\n";
            help += "/status - стан системи\n";
            bot.sendMessage(ADMIN_CHAT_ID, help, "Markdown");

        } else if (text == "/list") {
            bot.sendMessage(ADMIN_CHAT_ID, listCards(), "Markdown");

        } else if (text == "/log") {
            bot.sendMessage(ADMIN_CHAT_ID, getLastLogs(10), "Markdown");

        } else if (text.startsWith("/delete ")) {
            String uid = text.substring(8);
            uid.trim(); uid.toUpperCase();
            if (removeCard(uid)) {
                bot.sendMessage(ADMIN_CHAT_ID, "Картку `" + uid + "` видалено.", "Markdown");
            } else {
                bot.sendMessage(ADMIN_CHAT_ID, "❌ Картку не знайдено: `" + uid + "`", "Markdown");
            }

        } else if (text == "/status") {
            String s = "⚙️ *Статус системи*\n";
            s += "📡 Wi-Fi: " + WiFi.localIP().toString() + "\n";
            s += "💾 SD: OK\n";
            s += "Uptime: " + String(millis() / 60000) + " хв";
            bot.sendMessage(ADMIN_CHAT_ID, s, "Markdown");

        } else {
            bot.sendMessage(ADMIN_CHAT_ID,
                "Невідома команда. Список команд /help", "");
        }
    }
}

static unsigned long lastPoll = 0;

void tgPoll() {
    if (!wifiConnected()) return;
    if (millis() - lastPoll < TELEGRAM_POLL_MS) return;
    lastPoll = millis();
    int n = bot.getUpdates(bot.last_message_received + 1);
    if (n > 0) tgHandleMessages(n);
}
