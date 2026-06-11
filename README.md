## ESP32-RFID---access-control
Система контролю доступу до приміщення на базі ESP32 та RFID.

Цей репозиторій є практичною реалізацією моєї дипломної роботи на тему: "Розробка системи контролю доступу до приміщення на базі ESP32".

## Index
* [Початок роботи](#Початок-роботи)
  * [Бібліотеки](#Бібліотеки)   
* [Основні файли](#Основні-файли)
  * [Access_control.ino](#access-control.ino)
  * [Config.h](#config.h)
  * [Buzzer.h](#buzzer.h)
  * [Display.h](#display.h)
  * [RFID.h](#rfid.h)
  * [Storage.h](#storage.h)
  * [Telegram.h](#telegram.h)
* [Тестування](#Тестування)
  * [Buzzer test](#Buzzer-test)
  * [RFID test](#RFID-test.ino)
  * [Telegram Bot test](#Telegram-bot-test.ino)

<p align="center">
  <img src="ESP32 AC.jpg" alt="ESP32 Access Control">
</p>

## Початок роботи:
# Бібліотеки
- MFRC522           by GithubCommunity
- ArduinoJson       by Benoit Blanchon  (v6.x)
- Adafruit GFX      by Adafruit
- Adafruit ST7789   by Adafruit
- UniversalTelegramBot by Brian Lough
- SD                (вбудована в ESP32 core)

[Повернутися на початок](#index)

## Основні файли:
# access_control.ino
[Повернутися на початок](#index)
# config.h
[Повернутися на початок](#index)
# buzzer.h
[Повернутися на початок](#index)
# display.h
[Повернутися на початок](#index)
# rfid.h
[Повернутися на початок](#index)
# storage.h
[Повернутися на початок](#index)
# telegram.h
[Повернутися на початок](#index)

## Тестування:
# Buzzer test.ino
Призначення: тестова прошивка для перевірки buzzer на правильність підключення та здатність працювати.

[Повернутися на початок](#index)

# RFID test.ino
Призначення: тестова прошивка для перевірки RFID на правильність підключення та здатність працювати.

[Повернутися на початок](#index)

# Telegram bot test.ino
Призначення: тестова прошивка для перевірки ESP32, модулю wi-fi на здатність працювати також перевірка правильності wi-fi ssid, wi-fi password, Telegram Chat ID, Bot token.

[Повернутися на початок](#index)

## Це не кінцевий варіант прошивки, будуть вноситися зміни.
