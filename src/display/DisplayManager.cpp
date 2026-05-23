#include <Wire.h>
#include "DisplayManager.h"

DisplayManager Display;

bool DisplayManager::begin() {
    // ── Backlight ──────────────────────────────────────────────────────────
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    // ── QSPI bus + AXS15231B panel ────────────────────────────────────────
    Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(
        LCD_CS, LCD_CLK, LCD_D0, LCD_D1, LCD_D2, LCD_D3
    );

    // Native panel: 172w × 640h (portrait). r=1 rotates to landscape 640×172.
    _panel = new Arduino_AXS15231B(bus, LCD_RST, 1 /*rotation*/, false /*IPS*/, 172, 640);

    if (!_panel->begin()) return false;

    _panel->fillScreen(COLOR_BG);

    // ── Touch I2C ──────────────────────────────────────────────────────────
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);

    return true;
}

// AXS15231B touch read: write 11-byte command, read 8 bytes back.
// Returns screen-space coordinates (already rotated for landscape).
bool DisplayManager::readTouchRaw(int16_t &sx, int16_t &sy) {
    // 1-point read command (1 × 6 + 2 = 8 bytes requested)
    const uint8_t cmd[11] = {
        0xb5, 0xab, 0xa5, 0x5a,
        0x00, 0x00, 0x00, 0x08,
        0x00, 0x00, 0x00
    };

    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(cmd, sizeof(cmd));
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom(TOUCH_ADDR, 8) < 8) return false;

    uint8_t d[8];
    for (int i = 0; i < 8; i++) d[i] = Wire.read();

    if (d[1] == 0) return false; // no touch points

    // Native coords (portrait 172×640)
    int16_t raw_x = (int16_t)(((d[2] & 0x0F) << 8) | d[3]);
    int16_t raw_y = (int16_t)(((d[4] & 0x0F) << 8) | d[5]);

    // Map to landscape screen coords (640×172):
    // portrait x [0,171] → landscape y; portrait y [0,639] → landscape x
    sx = TOUCH_MIRROR_X ? (639 - raw_y) : raw_y;
    sy = TOUCH_MIRROR_Y ? (171 - raw_x) : raw_x;

    return true;
}

TouchEvent DisplayManager::pollTouch() {
    TouchEvent evt;
    int16_t tx = 0, ty = 0;
    bool touching = readTouchRaw(tx, ty);
    uint32_t now = millis();

    if (touching && !_pressed) {
        _pressed     = true;
        _startX      = tx;
        _startY      = ty;
        _curX        = tx;
        _curY        = ty;
        _pressMs     = now;
        _longEmitted = false;
    } else if (touching && _pressed) {
        _curX = tx;
        _curY = ty;
        int16_t dx = _curX - _startX;
        if (!_longEmitted
            && (uint32_t)(now - _pressMs) >= LONG_PRESS_MS
            && abs(dx) < SWIPE_THRESHOLD_PX) {
            _longEmitted = true;
            evt.x = _startX;
            evt.y = _startY;
            evt.gesture = (_startX < SCREEN_W / 2)
                          ? TouchGesture::LONG_PRESS_LEFT
                          : TouchGesture::LONG_PRESS_RIGHT;
        }
    } else if (!touching && _pressed) {
        _pressed = false;
        if (!_longEmitted) {
            uint32_t dur = (uint32_t)(now - _pressMs);
            int16_t  dx  = _curX - _startX;
            if (dur <= TAP_MAX_MS && abs(dx) < SWIPE_THRESHOLD_PX) {
                evt.gesture = TouchGesture::TAP;
                evt.x = _startX;
                evt.y = _startY;
            } else if (abs(dx) >= SWIPE_THRESHOLD_PX) {
                evt.gesture = (dx < 0) ? TouchGesture::SWIPE_LEFT
                                        : TouchGesture::SWIPE_RIGHT;
                evt.x = _startX;
                evt.y = _startY;
            }
        }
    }

    return evt;
}
