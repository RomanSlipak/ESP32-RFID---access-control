/*
 * ═══════════════════════════════════════════════════
 *  Система контролю доступу на базі ESP32
 *  RFID · SD · TFT ST7789 · Telegram Bot
 * ═══════════════════════════════════════════════════
 *
 *  Бібліотеки:
 *  - MFRC522           by GithubCommunity
 *  - ArduinoJson       by Benoit Blanchon  (v6.x)
 *  - Adafruit GFX      by Adafruit
 *  - Adafruit ST7789   by Adafruit
 *  - UniversalTelegramBot by Brian Lough
 *  - SD                (вбудована в ESP32 core)
 */

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "buzzer.h"
#include "display.h"
#include "rfid.h"
#include "storage.h"
#include "telegram.h"


//  Стан замка

static unsigned long doorOpenedAt = 0;
static bool doorIsOpen = false;
bool sdAvailable = false;

void doorOpen() {
    digitalWrite(PIN_RELAY, RELAY_OPEN);
    doorIsOpen   = true;
    doorOpenedAt = millis();
    Serial.println("[DOOR] Відчинено");
}

void doorCheckClose() {
    if (doorIsOpen && millis() - doorOpenedAt >= DOOR_OPEN_MS) {
        digitalWrite(PIN_RELAY, RELAY_CLOSE);
        doorIsOpen = false;
        Serial.println("[DOOR] Зачинено");
        displayIdle();
    }
}

void setup() {
    Serial.begin(9600);
    delay(2000);
    Serial.println("\n═══ Access Control Boot ═══");

    // 1. Реле
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, RELAY_CLOSE);
    Serial.println("[SETUP] Relay OK");

    // 2. Buzzer
    Serial.println("[SETUP] Buzzer init...");
    buzzerInit();
    beepBoot();
    Serial.println("[SETUP] Buzzer OK");

    // 3. SPI + RFID
    Serial.println("[SETUP] SPI init...");
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
    Serial.println("[SETUP] RFID init...");
    rfidInit();
    Serial.println("[SETUP] RFID OK");

    // 4. SD карта
    Serial.println("[SETUP] SD init...");
    if (!storageInit()) {
        Serial.println("[WARN] SD FAILED - no SD mode");
        sdAvailable = false;
    } else {
        Serial.println("[SETUP] SD OK");
        sdAvailable = true;
    }

    // 5. Wi-Fi + Telegram
    Serial.println("[SETUP] WiFi connecting...");
    if (wifiConnect()) {
        Serial.println("[SETUP] WiFi OK");
        bot.sendMessage(ADMIN_CHAT_ID,
            "System started\nIP: " + WiFi.localIP().toString(),
            "Markdown");
    } else {
        Serial.println("[SETUP] WiFi FAILED");
    }

    Serial.println("═══ Ready ═══\n");
}

void loop() {
    //Автозакривання замка
    doorCheckClose();

    //Telegram
    tgPoll();

    // Зчитування RFID
    // Під час відкритого замка — не читати повторно
    if (doorIsOpen) return;

    if (!rfidCardPresent()) return;

    String uid = rfidReadUID();
    Serial.println("[RFID] UID: " + uid);

    // Пошук у базі
    Card card = findCard(uid);

    if (card.allowed) {
        // ДОСТУП ДОЗВОЛЕНО
        Serial.println("[ACCESS] GRANTED → " + String(card.name));

        displayGranted(String(card.name));
        beepSuccess();
        doorOpen();
        logEvent(uid, String(card.name), "GRANTED");
        tgNotifyGranted(String(card.name), uid);

    } else {
        // ДОСТУП ЗАБОРОНЕНО
        Serial.println("[ACCESS] DENIED → " + uid);

        displayDenied(uid);
        beepDenied();
        logEvent(uid, "UNKNOWN", "DENIED");
        tgNotifyUnknown(uid);

        delay(3000);
        displayIdle();
    }
}
