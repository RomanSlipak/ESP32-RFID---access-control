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
#define MASTER_CARD_UID  "E105FC03" 

//  Wi-Fi
#define WIFI_SSID        "Roiz's S23 hotspotu"
#define WIFI_PASSWORD    "22222222"
#define WIFI_TIMEOUT_MS  15000

//  Telegram
#define BOT_TOKEN        "6932525856:AAGaMaGvlnOhUjzNt31C0UOVY-ranGPq_Xc"
#define ADMIN_CHAT_ID    "2061951831"
#define TELEGRAM_POLL_MS 1000

//  Файли на SD
#define FILE_CARDS  "/cards.json"
#define FILE_LOG    "/log.csv"

//  Ліміти
#define MAX_CARDS  50
#define UID_LEN    9
#define NAME_LEN   32
