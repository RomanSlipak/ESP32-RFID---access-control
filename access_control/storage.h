#pragma once
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "config.h"

extern bool sdAvailable;

struct Card {
    char uid[UID_LEN];
    char name[NAME_LEN];
    bool allowed;
};

//  Ініціалізація SD
bool storageInit() {
    pinMode(RFID_CS, OUTPUT);
    digitalWrite(RFID_CS, HIGH); 
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    
    delay(10);
    
    if (!SD.begin(SD_CS, SPI, 1000000)) {
        Serial.println("[SD] Init failed. Card inserted? CS=GPIO27?");
        return false;
    }

    uint8_t t = SD.cardType();
    if (t == CARD_NONE) { Serial.println("[SD] No card."); return false; }

    Serial.printf("[SD] OK | Type: %s | Size: %llu MB\n",
        t == CARD_MMC  ? "MMC"  :
        t == CARD_SD   ? "SD"   :
        t == CARD_SDHC ? "SDHC" : "?",
        SD.cardSize() / (1024ULL * 1024ULL));

    if (!SD.exists(FILE_CARDS)) {
        File f = SD.open(FILE_CARDS, FILE_WRITE);
        if (f) { f.print("{\"cards\":[]}"); f.close(); }
        Serial.println("[SD] Created cards.json");
    }
    if (!SD.exists(FILE_LOG)) {
        File f = SD.open(FILE_LOG, FILE_WRITE);
        if (f) { f.println("timestamp,uid,name,result"); f.close(); }
        Serial.println("[SD] Created log.csv");
    }
    return true;
}

//  Пошук картки за UID
Card findCard(const String& uid) {
    Card result = {"", "", false};
    if (!sdAvailable) { Serial.println("[SD] Unavailable"); return result; }

    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) { Serial.println("[SD] Cannot open cards.json"); return result; }

    String json = "";
    while (f.available()) json += (char)f.read();
    f.close();

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) { Serial.println("[SD] JSON error: " + String(err.c_str())); return result; }

    for (JsonObject card : doc["cards"].as<JsonArray>()) {
        if (String((const char*)card["uid"]) == uid) {
            strlcpy(result.uid,  card["uid"]  | "", UID_LEN);
            strlcpy(result.name, card["name"] | "Unknown", NAME_LEN);
            result.allowed = true;
            Serial.println("[SD] Found: " + String(result.name));
            return result;
        }
    }
    Serial.println("[SD] Not found: " + uid);
    return result;
}

//  Додати картку
bool addCard(const String& uid, const String& name) {
    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return false;

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) return false;

    JsonArray cards = doc["cards"].as<JsonArray>();

    for (JsonObject c : cards)
        if (String((const char*)c["uid"]) == uid) return false;

    JsonObject nc = cards.createNestedObject();
    nc["uid"]  = uid;
    nc["name"] = name;

    SD.remove(FILE_CARDS);
    File out = SD.open(FILE_CARDS, FILE_WRITE);
    if (!out) return false;

    serializeJson(doc, out);
    out.close();

    Serial.println("[SD] Added: " + name + " (" + uid + ")");
    return true;
}

//  Видалити картку
bool removeCard(const String& uid) {
    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return false;

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) return false;

    JsonArray cards = doc["cards"].as<JsonArray>();
    bool found = false;

    for (int i = (int)cards.size() - 1; i >= 0; i--) {
        if (String((const char*)cards[i]["uid"]) == uid) {
            cards.remove(i);
            found = true;
            break;
        }
    }

    if (!found) return false;

    SD.remove(FILE_CARDS);
    File out = SD.open(FILE_CARDS, FILE_WRITE);
    if (!out) return false;

    serializeJson(doc, out);
    out.close();

    Serial.println("[SD] Removed: " + uid);
    return true;
}
//  Список карток > Telegram
String listCards() {
    if (!sdAvailable) return "❌ SD недоступна";

    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return "❌ Помилка читання";

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) return "❌ JSON error";

    JsonArray cards = doc["cards"].as<JsonArray>();
    if (cards.size() == 0) return "📋 Список порожній";

    String r = "📋 *Картки:*\n";
    int i = 1;

    for (JsonObject c : cards) {
        r += String(i++) + ". *" + String((const char*)c["name"]) + "*\n";
        r += "   `" + String((const char*)c["uid"]) + "`\n";
    }

    return r;
}

//  Запис події в лог
void logEvent(const String& uid, const String& name, const String& result) {
    if (!sdAvailable) return;
    File f = SD.open(FILE_LOG, FILE_APPEND);
    if (!f) { Serial.println("[LOG] Cannot open log.csv"); return; }

    unsigned long t = millis() / 1000;
    char ts[12];
    snprintf(ts, sizeof(ts), "%02lu:%02lu:%02lu",
             t / 3600, (t % 3600) / 60, t % 60);
    f.printf("%s,%s,%s,%s\n", ts, uid.c_str(), name.c_str(), result.c_str());
    f.close();
    Serial.printf("[LOG] %s %s -> %s\n", ts, uid.c_str(), result.c_str());
}

//  Останні N записів > Telegram
String getLastLogs(int count = 10) {
    if (!sdAvailable) return "❌ SD недоступна";
    File f = SD.open(FILE_LOG, FILE_READ);
    if (!f) return "❌ Лог недоступний";

    String lines[10];
    int idx = 0, total = 0;
    String line = "";
    bool firstLine = true;

    while (f.available()) {
        char c = f.read();
        if (c == '\n') {
            if (firstLine) { firstLine = false; line = ""; continue; }
            if (line.length() > 0) { lines[idx % count] = line; idx++; total++; }
            line = "";
        } else if (c != '\r') {
            line += c;
        }
    }
    f.close();

    if (total == 0) return "📜 Лог порожній";

    String out = "📜 *Останні події:*\n";
    int start = (total > count) ? idx % count : 0;
    int n = min(total, count);
    for (int i = 0; i < n; i++) {
        String& l = lines[(start + i) % count];
        int c1 = l.indexOf(','), c2 = l.indexOf(',', c1+1), c3 = l.indexOf(',', c2+1);
        if (c1 < 0 || c2 < 0 || c3 < 0) continue;
        String res  = l.substring(c3 + 1);
        String icon = (res.indexOf("GRANTED") >= 0) ? "✅" : "❌";
        out += icon + " " + l.substring(0, c1) + " — " + l.substring(c2+1, c3) + "\n";
    }
    return out;
}
