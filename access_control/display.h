#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Кольори
#define CLR_BG      0x0000
#define CLR_GREEN   0x07E0
#define CLR_RED     0xF800
#define CLR_WHITE   0xFFFF
#define CLR_YELLOW  0xFFE0
#define CLR_GRAY    0x7BEF

// Ініціалізація
void displayInit() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);  // підсвітка увімкнена

    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(1);          // горизонтально, якщо потрібно, зміни на 0..3
    tft.fillScreen(CLR_BG);

    tft.setTextColor(CLR_WHITE);
    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.println("Access Control");
    tft.setTextSize(1);
    tft.setCursor(55, 130);
    tft.setTextColor(CLR_GRAY);
    tft.println("Hold the card up...");

    Serial.println("[TFT] OK");
}

// Екран очікування
void displayIdle() {
    tft.fillScreen(CLR_BG);
    tft.setTextColor(CLR_WHITE);
    tft.setTextSize(2);
    tft.setCursor(25, 90);
    tft.println("Access Control");
    tft.setTextSize(1);
    tft.setTextColor(CLR_GRAY);
    tft.setCursor(50, 125);
    tft.println("Hold the card up...");
    
    // Маленька іконка картки
    tft.drawRect(90, 150, 60, 40, CLR_GRAY);
    tft.drawLine(90, 165, 150, 165, CLR_GRAY);
    tft.fillRect(95, 155, 15, 8, CLR_GRAY);
}

// Доступ дозволено
void displayGranted(const String& name) {
    tft.fillScreen(CLR_BG);

    // Зелений заголовок
    tft.fillRect(0, 0, TFT_WIDTH, 60, CLR_GREEN);
    tft.setTextColor(CLR_BG);
    tft.setTextSize(2);
    tft.setCursor(35, 20);
    tft.println("ACCESS OK");

    // Ім'я
    tft.setTextColor(CLR_WHITE);
    tft.setTextSize(2);
    // Центрування тексту
    int16_t x = (TFT_WIDTH - name.length() * 12) / 2;
    if (x < 5) x = 5;
    tft.setCursor(x, 90);
    tft.println(name);

    // Зелена галочка
    tft.drawLine(80, 155, 100, 175, CLR_GREEN); tft.drawLine(81, 155, 101, 175, CLR_GREEN);
    tft.drawLine(100, 175, 140, 135, CLR_GREEN); tft.drawLine(101, 175, 141, 135, CLR_GREEN);
}

//  Доступ заборонено
void displayDenied(const String& uid) {
    tft.fillScreen(CLR_BG);

    // Червоний заголовок
    tft.fillRect(0, 0, TFT_WIDTH, 60, CLR_RED);
    tft.setTextColor(CLR_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 20);
    tft.println("ACCESS X");

    tft.setTextColor(CLR_GRAY);
    tft.setTextSize(1);
    tft.setCursor(20, 80);
    tft.println("Unknown card:");
    tft.setTextColor(CLR_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.println(uid);

    tft.setTextColor(CLR_GRAY);
    tft.setTextSize(1);
    tft.setCursor(10, 145);
    tft.println("Message sent");
    tft.setCursor(10, 160);
    tft.println("to the administrator");
}

// Статус Wi-Fi / завантаження
void displayStatus(const String& msg, uint16_t color = CLR_GRAY) {
    tft.fillRect(0, TFT_HEIGHT - 25, TFT_WIDTH, 25, CLR_BG);
    tft.setTextColor(color);
    tft.setTextSize(1);
    tft.setCursor(5, TFT_HEIGHT - 15);
    tft.println(msg);
}

void displayBooting(const String& step) {
    static int y = 20;
    tft.setTextColor(CLR_WHITE);
    tft.setTextSize(1);
    tft.setCursor(5, y);
    tft.println(step);
    Serial.println("[DISPLAY] " + step);
    y += 14;
    if (y > TFT_HEIGHT - 14) {
        tft.fillScreen(CLR_BG);
        y = 20;
    }
}
