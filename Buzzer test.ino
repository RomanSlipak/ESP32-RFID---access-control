#include <Arduino.h>
#include "config.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n buzzer test PIN 21");
    
    pinMode(PIN_BUZZER, OUTPUT);
    Serial.println("pinMode OK");
}

void loop() {
    Serial.println("digitalWrite HIGH");
    digitalWrite(PIN_BUZZER, HIGH);
    delay(1000);
    
    Serial.println("digitalWrite LOW");
    digitalWrite(PIN_BUZZER, LOW);
    delay(1000);
}
