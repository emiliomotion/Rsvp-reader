#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "DisplayManager.h"

// ── LovyanGFX driver class ────────────────────────────────────────────────
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7796  _panel;
    lgfx::Bus_SPI       _bus;
    lgfx::Light_PWM     _light;
    lgfx::Touch_FT5x06  _touch;   // FT6336 is FT5x06 family

public:
    LGFX() {
        {
            auto cfg         = _bus.config();
            cfg.spi_host     = SPI2_HOST;
            cfg.spi_mode     = 0;
            cfg.freq_write   = 40000000;
            cfg.freq_read    = 16000000;
            cfg.spi_3wire    = false;
            cfg.use_lock     = true;
            cfg.dma_channel  = SPI_DMA_CH_AUTO;
            cfg.pin_sclk     = LCD_SCLK;
            cfg.pin_mosi     = LCD_MOSI;
            cfg.pin_miso     = LCD_MISO;
            cfg.pin_dc       = LCD_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg             = _panel.config();
            cfg.pin_cs           = LCD_CS;
            cfg.pin_rst          = LCD_RST;
            cfg.pin_busy         = -1;
            cfg.panel_width      = 320;
            cfg.panel_height     = 480;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 1;    // landscape: 480×320
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true; // SD_MMC shares bus implicitly; keeps CS mutex active
            _panel.config(cfg);
        }
        {
            auto cfg        = _light.config();
            cfg.pin_bl      = LCD_BL;
            cfg.invert      = false;
            cfg.freq        = 12000;
            cfg.pwm_channel = 7;
            _light.config(cfg);
            _panel.setLight(&_light);
        }
        {
            auto cfg             = _touch.config();
            cfg.x_min            = 0;
            cfg.x_max            = 319;
            cfg.y_min            = 0;
            cfg.y_max            = 479;
            cfg.pin_int          = TOUCH_INT;
            cfg.pin_rst          = TOUCH_RST;
            cfg.bus_shared       = false;
            cfg.offset_rotation  = 1;
            cfg.i2c_port         = 0;
            cfg.i2c_addr         = 0x38;
            cfg.pin_sda          = TOUCH_SDA;
            cfg.pin_scl          = TOUCH_SCL;
            cfg.freq             = 400000;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};

static LGFX _lgfx;

DisplayManager Display;

bool DisplayManager::begin() {
    _lcd = &_lgfx;
    _lgfx.init();
    _lgfx.setRotation(1);
    _lgfx.setBrightness(200);
    _lgfx.fillScreen(COLOR_BG);

    _sprite = new LGFX_Sprite(_lcd);
    _sprite->setPsram(true);
    _sprite->setColorDepth(16);
    if (!_sprite->createSprite(SCREEN_W, SCREEN_H)) {
        return false;
    }
    _sprite->fillScreen(COLOR_BG);
    return true;
}

LGFX_Sprite* DisplayManager::sprite() {
    return _sprite;
}

void DisplayManager::push() {
    _sprite->pushSprite(0, 0);
}

TouchEvent DisplayManager::pollTouch() {
    TouchEvent evt;
    lgfx::touch_point_t tp;
    bool touching = _lcd->getTouch(&tp) > 0;

    uint32_t now = millis();

    if (touching && !_pressed) {
        _pressed     = true;
        _startX      = tp.x;
        _startY      = tp.y;
        _curX        = tp.x;
        _curY        = tp.y;
        _pressMs     = now;
        _longEmitted = false;
    } else if (touching && _pressed) {
        _curX = tp.x;
        _curY = tp.y;
        int16_t dx = _curX - _startX;
        int displacement = abs(dx);
        if (!_longEmitted && (uint32_t)(now - _pressMs) >= LONG_PRESS_MS
                          && displacement < SWIPE_THRESHOLD_PX) {
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
            uint32_t duration = (uint32_t)(now - _pressMs);
            int16_t  dx       = _curX - _startX;
            if (duration <= TAP_MAX_MS && abs(dx) < SWIPE_THRESHOLD_PX) {
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
