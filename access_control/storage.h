#pragma once
#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "config.h"

// Зовнішня змінна для перевірки доступності SD
extern bool sdAvailable;

// Структура картки
struct Card {
    char uid[UID_LEN];
    char name[NAME_LEN];
    bool allowed;
};

// Ініціалізація SD
bool storageInit() {
    // Перед init SD переконатись, що інші CS = HIGH
    pinMode(RFID_CS, OUTPUT); digitalWrite(RFID_CS, HIGH);
    pinMode(TFT_CS,  OUTPUT); digitalWrite(TFT_CS,  HIGH);

    Serial.println("[SD] Спробуємо SD.begin(27)...");
    if (SD.begin(27)) {
        Serial.println("[SD] ✓ Успішно SD.begin(27)");
    } else {
        Serial.println("[SD] Не пройшло, спробуємо з SPI...");
        if (!SD.begin(27, SPI, 4000000)) {
            Serial.println("[SD] ✗ Дві спроби не спрацювали!");
            return false;
        }
        Serial.println("[SD] ✓ Успішно з SPI параметром");
    }

    // Створюємо файли, якщо немає
    if (!SD.exists(FILE_CARDS)) {
        File f = SD.open(FILE_CARDS, FILE_WRITE);
        if (f) { f.print("{\"cards\":[]}"); f.close(); }
    }
    if (!SD.exists(FILE_LOG)) {
        File f = SD.open(FILE_LOG, FILE_WRITE);
        if (f) { f.println("timestamp,uid,name,result"); f.close(); }
    }
    return true;
}

// Простеньке сховище для карток в пам'яті (без SD)
#define MAX_CARDS_MEMORY 20
struct MemCard {
    String uid;
    String name;
};
MemCard memCards[MAX_CARDS_MEMORY];
int memCardsCount = 0;

Card findCard(const String& uid) {
    Card result = {"", "", false};

    // Якщо є SD, шукаємо там
    if (sdAvailable) {
        File f = SD.open(FILE_CARDS, FILE_READ);
        if (!f) {
            Serial.println("[SD] Cannot open cards.json");
            return result;
        }

        String json = "";
        while (f.available()) json += (char)f.read();
        f.close();

        StaticJsonDocument<4096> doc;
        if (deserializeJson(doc, json) != DeserializationError::Ok) return result;

        JsonArray cards = doc["cards"];
        for (JsonObject card : cards) {
            if (String((const char*)card["uid"]) == uid) {
                strlcpy(result.uid,  card["uid"]  | "", UID_LEN);
                strlcpy(result.name, card["name"] | "Unknown", NAME_LEN);
                result.allowed = true;
                return result;
            }
        }
    } else {
        // Шукаємо в пам'яті
        for (int i = 0; i < memCardsCount; i++) {
            if (memCards[i].uid == uid) {
                strlcpy(result.uid,  memCards[i].uid.c_str(),  UID_LEN);
                strlcpy(result.name, memCards[i].name.c_str(), NAME_LEN);
                result.allowed = true;
                Serial.println("[MEM] Card found in memory: " + memCards[i].name);
                return result;
            }
        }
    }

    return result;
}

bool addCard(const String& uid, const String& name) {
    if (sdAvailable) {
        // Код для SD
        return false;  // TODO
    } else {
        // Для пам'яті
        if (memCardsCount >= MAX_CARDS_MEMORY) {
            Serial.println("[MEM] Memory full!");
            return false;
        }
        for (int i = 0; i < memCardsCount; i++) {
            if (memCards[i].uid == uid) {
                Serial.println("[MEM] Card already exists!");
                return false;
            }
        }
        memCards[memCardsCount].uid = uid;
        memCards[memCardsCount].name = name;
        memCardsCount++;
        Serial.println("[MEM] Card added: " + name);
        return true;
    }
}

// Додати нову картку
bool addCard(const String& uid, const String& name) {
    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return false;
    String json = "";
    while (f.available()) json += (char)f.read();
    f.close();

    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;

    // Перевірка на дублікат
    JsonArray cards = doc["cards"];
    for (JsonObject card : cards) {
        if (String((const char*)card["uid"]) == uid) return false;
    }

    JsonObject newCard = cards.createNestedObject();
    newCard["uid"]  = uid;
    newCard["name"] = name;

    // Записуємо назад
    SD.remove(FILE_CARDS);
    File out = SD.open(FILE_CARDS, FILE_WRITE);
    if (!out) return false;
    serializeJson(doc, out);
    out.close();
    return true;
}

// ─────────────────────────────────────────────
//  Видалити картку
// ─────────────────────────────────────────────
bool removeCard(const String& uid) {
    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return false;
    String json = "";
    while (f.available()) json += (char)f.read();
    f.close();

    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return false;

    JsonArray cards = doc["cards"];
    bool found = false;
    for (int i = cards.size() - 1; i >= 0; i--) {
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
    return true;
}

// ─────────────────────────────────────────────
//  Список всіх карток (повертає рядок для Telegram)
// ─────────────────────────────────────────────
String listCards() {
    File f = SD.open(FILE_CARDS, FILE_READ);
    if (!f) return "Помилка читання SD";
    String json = "";
    while (f.available()) json += (char)f.read();
    f.close();

    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return "Помилка JSON";

    JsonArray cards = doc["cards"];
    if (cards.size() == 0) return "Список порожній";

    String result = "📋 *Список карток:*\n";
    int i = 1;
    for (JsonObject card : cards) {
        result += String(i++) + ". ";
        result += String((const char*)card["name"]) + "\n";
        result += "   UID: `" + String((const char*)card["uid"]) + "`\n";
    }
    return result;
}

// ─────────────────────────────────────────────
//  Записати подію у лог
// ─────────────────────────────────────────────
void logEvent(const String& uid, const String& name, const String& result) {
    if (!sdAvailable) {
        Serial.println("[LOG] SD недоступна — лог не записаний");
        return;
    }

    File f = SD.open(FILE_LOG, FILE_APPEND);
    if (!f) {
        Serial.println("[LOG] Помилка: не можу відкрити файл логу!");
        return;
    }

    // Час з millis() — якщо є NTP, можна замінити на реальний час
    unsigned long t = millis() / 1000;
    char ts[20];
    snprintf(ts, sizeof(ts), "%02lu:%02lu:%02lu", t/3600, (t%3600)/60, t%60);

    int written = f.printf("%s,%s,%s,%s\n", ts, uid.c_str(), name.c_str(), result.c_str());
    f.close();
    
    if (written > 0) {
        Serial.println("[LOG] OK: " + String(uid) + " → " + result);
    } else {
        Serial.println("[LOG] Помилка при записі в файл!");
    }
}

// ─────────────────────────────────────────────
//  Останні N записів логу для Telegram
// ─────────────────────────────────────────────
String getLastLogs(int count = 10) {
    File f = SD.open(FILE_LOG, FILE_READ);
    if (!f) return "Лог недоступний";

    // Зберігаємо останні count рядків
    String lines[10];
    int idx = 0, total = 0;
    String line = "";
    bool firstLine = true;

    while (f.available()) {
        char c = f.read();
        if (c == '\n') {
            if (firstLine) { firstLine = false; line = ""; continue; } // пропуск заголовка
            if (line.length() > 0) {
                lines[idx % count] = line;
                idx++; total++;
            }
            line = "";
        } else {
            line += c;
        }
    }
    f.close();

    if (total == 0) return "Лог порожній";

    String out = "📜 *Останні події:*\n";
    int start = (total > count) ? idx % count : 0;
    int n = min(total, count);
    for (int i = 0; i < n; i++) {
        String& l = lines[(start + i) % count];
        // Парсимо CSV: час,uid,ім'я,результат
        int c1 = l.indexOf(','), c2 = l.indexOf(',', c1+1),
            c3 = l.indexOf(',', c2+1);
        String res = l.substring(c3+1);
        String icon = (res == "GRANTED") ? "✅" : "❌";
        out += icon + " " + l.substring(0, c1) + " — " +
               l.substring(c2+1, c3) + "\n";
    }
    return out;
}
