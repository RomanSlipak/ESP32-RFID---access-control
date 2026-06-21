#pragma once
#include <Arduino.h>
#include "config.h"

#define BUZZER_FREQ 1000
#define BUZZER_BITS 8

void buzzerInit() {
    ledcAttach(PIN_BUZZER, BUZZER_FREQ, BUZZER_BITS);
    Serial.println("[BUZZER] OK");
}

void _beep(int freq, int ms) {
    ledcWriteTone(PIN_BUZZER, freq);
    delay(ms);
    ledcWriteTone(PIN_BUZZER, 0);  // Вимкнути
}

void beepSuccess() {
    _beep(1000, 100);
    delay(50);
    _beep(1500, 150);
}

void beepDenied() {
    _beep(400, 400);
}

void beepBoot() {
    for (int i = 0; i < 3; i++) {
        _beep(800, 80);
        delay(60);
    }
}
