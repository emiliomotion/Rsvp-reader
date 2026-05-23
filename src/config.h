#pragma once

// ── Pin assignments ────────────────────────────────────────────────────────
// Verify all GPIO numbers against the schematic before flashing:
// https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-3.5/ESP32-S3-Touch-LCD-3.5-Schematic.pdf

#define LCD_MOSI    47
#define LCD_MISO    41
#define LCD_SCLK    48
#define LCD_CS      45
#define LCD_DC      40
#define LCD_RST     39
#define LCD_BL      21

#define TOUCH_SDA    8
#define TOUCH_SCL    9
#define TOUCH_INT    3   // set to -1 if INT pin unconfirmed
#define TOUCH_RST   46   // set to -1 if RST unconnected

#define SD_CLK      14
#define SD_CMD      13
#define SD_DATA0    12

// ── Display geometry (landscape 480×320) ──────────────────────────────────
#define SCREEN_W     480
#define SCREEN_H     320
#define STATUS_H      30
#define CONTROLS_H    70
#define READING_Y     STATUS_H
#define READING_H    (SCREEN_H - STATUS_H - CONTROLS_H)  // 220
#define PROGRESS_H     4
#define PROGRESS_Y    (SCREEN_H - CONTROLS_H - PROGRESS_H)

// ── Colors (RGB565) ───────────────────────────────────────────────────────
#define COLOR_BG       0x0000   // black
#define COLOR_TEXT     0xFFFF   // white
#define COLOR_ORP      0xF800   // red
#define COLOR_STATUS   0x7BEF   // light grey
#define COLOR_PROGRESS 0x07E0   // green
#define COLOR_PROGRESS_BG 0x39E7
#define COLOR_BTN      0x2945   // dark grey button
#define COLOR_BTN_ACT  0x4A69   // active button

// ── RSVP parameters ───────────────────────────────────────────────────────
#define WPM_MIN        100
#define WPM_MAX       1000
#define WPM_DEFAULT    300
#define WPM_STEP        25
#define ORP_RATIO     0.3f
#define SEEK_WORDS      10   // words to jump per swipe

// ── Touch thresholds ──────────────────────────────────────────────────────
#define SWIPE_THRESHOLD_PX  40
#define LONG_PRESS_MS      600
#define TAP_MAX_MS         200
#define DEBOUNCE_MS         80
