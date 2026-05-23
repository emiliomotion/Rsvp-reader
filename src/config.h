#pragma once

// ── Pin assignments ────────────────────────────────────────────────────────
// Source: Waveshare ESP32-S3-Touch-LCD-3.49 official repo
// https://github.com/waveshareteam/ESP32-S3-Touch-LCD-3.49

// QSPI display (AXS15231B)
#define LCD_CS    9
#define LCD_CLK   10
#define LCD_D0    11
#define LCD_D1    12
#define LCD_D2    13
#define LCD_D3    14
#define LCD_RST   21
#define LCD_BL    8

// Touch (AXS15231B I2C, integrated with display driver)
#define TOUCH_SDA   17
#define TOUCH_SCL   18
#define TOUCH_ADDR  0x3B

// SD card (SDMMC 1-bit mode)
#define SD_CLK    41
#define SD_CMD    39
#define SD_DATA0  40

// Internal I2C bus (QMI8658 IMU + PCF85063 RTC — unused by this app)
// #define INT_SDA  47
// #define INT_SCL  48

// ── Display geometry (landscape 640×172) ──────────────────────────────────
#define SCREEN_W     640
#define SCREEN_H     172

#define STATUS_H      22
#define CONTROLS_H    46
#define PROGRESS_H     4

#define READING_Y     STATUS_H
#define READING_H    (SCREEN_H - STATUS_H - CONTROLS_H - PROGRESS_H)  // 100
#define PROGRESS_Y   (STATUS_H + READING_H)                            // 122
#define CONTROLS_Y   (PROGRESS_Y + PROGRESS_H)                         // 126

// Touch native resolution (portrait: 172×640)
// After landscape rotation: screen_x = raw_y, screen_y = raw_x
// Mirror as needed based on physical board orientation:
#define TOUCH_MIRROR_X  false   // set true if X is flipped
#define TOUCH_MIRROR_Y  false   // set true if Y is flipped

// ── Colors (RGB565) ───────────────────────────────────────────────────────
#define COLOR_BG           0x0000   // black
#define COLOR_TEXT         0xFFFF   // white
#define COLOR_ORP          0xF800   // red
#define COLOR_STATUS_BG    0x2104   // very dark grey
#define COLOR_STATUS_TEXT  0xC618   // light grey
#define COLOR_PROGRESS     0x07E0   // green
#define COLOR_PROGRESS_BG  0x39E7   // dark green-grey
#define COLOR_BTN          0x2945   // dark button
#define COLOR_BTN_ACTIVE   0x4A69   // lit button

// ── RSVP parameters ───────────────────────────────────────────────────────
#define WPM_MIN        100
#define WPM_MAX       1000
#define WPM_DEFAULT    300
#define WPM_STEP        25
#define ORP_RATIO     0.3f
#define SEEK_WORDS      10

// ── Touch gesture thresholds ──────────────────────────────────────────────
#define SWIPE_THRESHOLD_PX  40
#define LONG_PRESS_MS      600
#define TAP_MAX_MS         200
