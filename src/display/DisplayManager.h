#pragma once

#include <Arduino_GFX_Library.h>
#include "../config.h"

enum class TouchGesture : uint8_t {
    NONE,
    TAP,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    LONG_PRESS_LEFT,
    LONG_PRESS_RIGHT
};

struct TouchEvent {
    TouchGesture gesture = TouchGesture::NONE;
    int16_t      x       = 0;
    int16_t      y       = 0;
};

class DisplayManager {
public:
    bool begin();

    Arduino_GFX* gfx() { return _panel; }
    void         push() {}   // no-op: direct-to-panel drawing, kept for API compat

    TouchEvent   pollTouch();

private:
    Arduino_AXS15231B* _panel = nullptr;

    bool     _pressed     = false;
    int16_t  _startX      = 0;
    int16_t  _startY      = 0;
    int16_t  _curX        = 0;
    int16_t  _curY        = 0;
    uint32_t _pressMs     = 0;
    bool     _longEmitted = false;

    bool readTouchRaw(int16_t &sx, int16_t &sy);
};

extern DisplayManager Display;
