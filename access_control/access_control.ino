/*
 * ═══════════════════════════════════════════════════
 *  Система контролю доступу — ESP32 Arduino Core v3.x
 * ═══════════════════════════════════════════════════
 *  Tools → Board        → ESP32 Dev Module
 *  Serial Monitor       → 115200 baud
 */

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "buzzer.h"
#include "display.h"
#include "rfid.h"
#include "storage.h"
#include "telegram.h"

bool sdAvailable = false;

//  Замок
static unsigned long doorOpenedAt = 0;
static bool          doorIsOpen   = false;

void doorOpen() {
    digitalWrite(PIN_RELAY, RELAY_OPEN);

    Serial.print("[DOOR] GPIO13=");
    Serial.println(digitalRead(PIN_RELAY));

    doorIsOpen = true;
    doorOpenedAt = millis();

    Serial.println("[DOOR] Opened");
}

void doorCheckClose() {
    if (doorIsOpen && millis() - doorOpenedAt >= DOOR_OPEN_MS) {
        digitalWrite(PIN_RELAY, RELAY_CLOSE);
        doorIsOpen = false;
        Serial.println("[DOOR] Closed");
        digitalWrite(SD_CS,   HIGH);
        digitalWrite(RFID_CS, HIGH);
    }
}

//  Майстер картка
bool isMasterCard(const String& uid) {
    return uid.equalsIgnoreCase(MASTER_CARD_UID);
}

void releaseSpiBus() {
    digitalWrite(SD_CS,   HIGH);
    digitalWrite(RFID_CS, HIGH);
    delayMicroseconds(50);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\n=== Access Control Boot ===");

    // 1. Реле > ЗАКРИТО
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, RELAY_CLOSE);

    // 2. Всі CS > HIGH до SPI.begin()
    pinMode(RFID_CS, OUTPUT); digitalWrite(RFID_CS, HIGH);
    pinMode(SD_CS, OUTPUT);   digitalWrite(SD_CS, HIGH);
    
    delay(100);

    // 3. Зумер
    buzzerInit();

    // 4. SPI — один раз
    SPI.end();
    delay(20);
    Serial.println("A: before SPI.begin");
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);
    Serial.println("B: after SPI.begin");
    delay(50);
    releaseSpiBus();

    // 5. RFID
    rfidInit();
    Serial.println("[RFID] test...");
    digitalWrite(RFID_CS, LOW);
    
    byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    
    digitalWrite(RFID_CS, HIGH);
    
    Serial.printf("[RFID] Version raw = 0x%02X\n", v);
    releaseSpiBus();
   
    // 6. SD
    sdAvailable = storageInit();
    releaseSpiBus();
   
    // 7. WiFi + Telegram
    if (wifiConnect()) {
        releaseSpiBus();
        bot.sendMessage(ADMIN_CHAT_ID,
            "🟢 *Система запущена*\nIP: " + WiFi.localIP().toString(),
            "Markdown");
    } else {
        releaseSpiBus();
    }

    delay(100);
    beepBoot();
    releaseSpiBus();
    Serial.println("=== System ready ===\n");
}

void loop() {
    doorCheckClose();
    tgPoll();
    
    if (doorIsOpen) return;

    static unsigned long t = 0;
    
    if (millis() - t > 1000) {
        t = millis();
        Serial.println("[RFID] scan...");
    }

    if (!rfidCardPresent()) return;

    String uid = rfidReadUID();
    Serial.println("[RFID] UID: " + uid);

    // Майстер картка
    if (isMasterCard(uid)) {
        Serial.println("[ACCESS] MASTER");
        releaseSpiBus();
        beepSuccess();
        doorOpen();
        logEvent(uid, "MASTER", "GRANTED");
        tgNotifyGranted("MASTER", uid);
        return;
    }

    // Звичайна картка
    Card card = findCard(uid);
    releaseSpiBus();

    if (card.allowed) {
        Serial.println("[ACCESS] GRANTED -> " + String(card.name));
        beepSuccess();
        doorOpen();
        logEvent(uid, String(card.name), "GRANTED");
        tgNotifyGranted(String(card.name), uid);
    } else {
        Serial.println("[ACCESS] DENIED -> " + uid);
        beepDenied();
        logEvent(uid, "UNKNOWN", "DENIED");
        tgNotifyUnknown(uid);
        delay(3000);
        releaseSpiBus();
    }
}
