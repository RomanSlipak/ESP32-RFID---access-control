#pragma once

//  Піни SPI (спільна шина VSPI)
#define PIN_SCK    18
#define PIN_MOSI   23
#define PIN_MISO   19

//  RFID RC522
#define RFID_CS    5
#define RFID_RST   15

//  SD-карта
#define SD_CS      27

//  Buzzer та реле
#define PIN_BUZZER   21
#define PIN_RELAY    13
#define RELAY_OPEN   LOW
#define RELAY_CLOSE  HIGH
#define DOOR_OPEN_MS 3000

//  Майстер картка
#define MASTER_CARD_UID  " - UID вашої майстер-карти - " 

//  Wi-Fi
#define WIFI_SSID        " - Ім'я вашого Wi-Fi - "
#define WIFI_PASSWORD    " - Пароль до Wi-Fi - "
#define WIFI_TIMEOUT_MS  15000

//  Telegram
#define BOT_TOKEN        " - Telegram bot token - "
#define ADMIN_CHAT_ID    " - Telegram user ID - "
#define TELEGRAM_POLL_MS 1000

//  Файли на SD
#define FILE_CARDS  "/cards.json"
#define FILE_LOG    "/log.csv"

//  Ліміти
#define MAX_CARDS  50
#define UID_LEN    9
#define NAME_LEN   32
