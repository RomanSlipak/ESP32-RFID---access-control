## ESP32-RFID---access-control
Система контролю доступу до приміщення на базі ESP32 та RFID.

Цей репозиторій є практичною реалізацією моєї дипломної роботи на тему: "Розробка системи контролю доступу до приміщення на базі ESP32".

## Зміст
* [Початок роботи](#Початок-роботи)
  * [Бібліотеки](#Бібліотеки)   
* [Основні файли](#Основні-файли)
  * [Access_control.ino](#Access_control)
  * [Config.h](#Config)
  * [Buzzer.h](#Buzzer)
  * [Display.h](#Display)
  * [RFID.h](#RFID)
  * [Storage.h](#Storage)
  * [Telegram.h](#Telegram)
* [Тестування](#Тестування)
  * [Buzzer test.ino](#Buzzer-test)
  * [RFID test.ino](#RFID-test)
  * [Telegram Bot test.ino](#Telegram-bot-test)

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
# Access_control
Назва файлу: access_control.ino

[Повернутися на початок](#index)

# Config
Назва файлу: config.h

[Повернутися на початок](#index)

# Buzzer
Назва файлу: buzzer.h

[Повернутися на початок](#index)

# Display
Назва файлу: display.h

[Повернутися на початок](#index)

# RFID
Назва файлу: rfid.h

[Повернутися на початок](#index)

# Storage
Назва файлу: storage.h

[Повернутися на початок](#index)

# Telegram
Назва файлу: telegram.h

[Повернутися на початок](#index)

## Тестування:
# Buzzer test
Назва файлу: Buzzer test.ino
Призначення: тестова прошивка для перевірки buzzer на правильність підключення та здатність працювати.

[Повернутися на початок](#index)

# RFID test
Назва файлу: RFID test.ino
Призначення: тестова прошивка для перевірки RFID на правильність підключення та здатність працювати.

[Повернутися на початок](#index)

# Telegram bot test
Назва файлу: Telegram bot test.ino
Призначення: тестова прошивка для перевірки ESP32, модулю wi-fi на здатність працювати також перевірка правильності wi-fi ssid, wi-fi password, Telegram Chat ID, Bot token.

[Повернутися на початок](#index)

## Це не кінцевий варіант прошивки, будуть вноситися зміни.
