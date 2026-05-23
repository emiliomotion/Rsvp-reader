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

// Internal I2C bus (shared by IMU, RTC, audio codec, and TCA9554 I/O expander)
#define IEXP_SDA       47
#define IEXP_SCL       48
#define IEXP_ADDR      0x20    // TCA9554PWR with A0-A2 grounded
#define IEXP_REG_OUT   0x01
#define IEXP_REG_CFG   0x03

// I/O expander pin assignments (from schematic GPIO table):
//   P0=TP_INT   P1=BL_EN    P2=IMU_INT1  P3=IMU_INT2
//   P4=RTC_INT  P5=LCD_TE   P6=SYS_EN    P7=NS_MODE
// Outputs we drive: BL_EN (P1) and SYS_EN (P6) high to power the display.
#define IEXP_BIT_BL_EN   (1 << 1)
#define IEXP_BIT_SYS_EN  (1 << 6)
// Configure P1+P6+P7 as outputs (0), everything else as input (1):
#define IEXP_CFG_VALUE   0x3D    // 0b00111101
#define IEXP_OUT_VALUE   (IEXP_BIT_BL_EN | IEXP_BIT_SYS_EN)  // 0x42

// SD card (SDMMC 1-bit mode)
#define SD_CLK    41
#define SD_CMD    39
#define SD_DATA0  40

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
