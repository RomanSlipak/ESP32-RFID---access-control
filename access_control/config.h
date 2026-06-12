#pragma once

// SPI
#define PIN_SCK    18
#define PIN_MOSI   23
#define PIN_MISO   19

// RFID RC522
#define RFID_CS    5
#define RFID_RST   15

// SD-карта
#define SD_CS      27

// TFT ST7789 (240x240)
#define TFT_CS     4
#define TFT_DC     2
#define TFT_RST    16
#define TFT_BL     17
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Buzzer та реле
#define PIN_BUZZER  21
#define PIN_RELAY   22
#define RELAY_OPEN  LOW
#define RELAY_CLOSE HIGH
#define DOOR_OPEN_MS 3000

// Wi-Fi
#define WIFI_SSID     "Roiz's S23 hotspotu"
#define WIFI_PASSWORD "22222222"
#define WIFI_TIMEOUT_MS 10000

// Telegram
#define BOT_TOKEN   "6932525856:AAF841c3DZRiL025CaQXF_R-yMceuav8X1E"
#define ADMIN_CHAT_ID "2061951831"
#define TELEGRAM_POLL_MS 1000

// Файли на SD
#define FILE_CARDS  "/cards.json"
#define FILE_LOG    "/log.csv"

// Ліміти
#define MAX_CARDS   50
#define UID_LEN     9
#define NAME_LEN    32
