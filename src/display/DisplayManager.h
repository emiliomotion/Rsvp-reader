#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
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

    LGFX_Sprite* sprite();   // back buffer — draw here each frame
    void         push();     // DMA-push sprite to display

    TouchEvent   pollTouch();

    lgfx::LGFX_Device& lcd() { return *_lcd; }

private:
    lgfx::LGFX_Device* _lcd    = nullptr;
    LGFX_Sprite*       _sprite = nullptr;

    // Touch gesture state
    bool     _pressed     = false;
    int16_t  _startX      = 0;
    int16_t  _startY      = 0;
    int16_t  _curX        = 0;
    int16_t  _curY        = 0;
    uint32_t _pressMs     = 0;
    bool     _longEmitted = false;
};

extern DisplayManager Display;
