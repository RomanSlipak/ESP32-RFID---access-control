#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"

MFRC522 mfrc522(RFID_CS, RFID_RST);

// Ініціалізація
void rfidInit() {
    mfrc522.PCD_Init();
    delay(50);
    Serial.print("[RFID] Версія: ");
    mfrc522.PCD_DumpVersionToSerial();
}

// Перевірка наявності картки
bool rfidCardPresent() {
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

// Зчитування UID
String rfidReadUID() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    mfrc522.PICC_HaltA();       // зупинити картку
    mfrc522.PCD_StopCrypto1();  // зупинити шифрування
    return uid;
}
