#include "Reader.h"

Reader ReaderUI;

// ── Status bar ─────────────────────────────────────────────────────────────
void Reader::drawStatusBar() {
    LGFX_Sprite* spr = Display.sprite();
    spr->fillRect(0, 0, SCREEN_W, STATUS_H, COLOR_STATUS);

    spr->setTextColor(COLOR_BG, COLOR_STATUS);
    spr->setTextSize(1);

    // WPM — left
    char buf[24];
    snprintf(buf, sizeof(buf), "%d WPM", Engine.wpm());
    spr->setCursor(6, (STATUS_H - 8) / 2);
    spr->print(buf);

    // Progress % — center
    uint8_t pct = Engine.progressPct();
    snprintf(buf, sizeof(buf), "%d%%", pct);
    int tw = strlen(buf) * 6;
    spr->setCursor((SCREEN_W - tw) / 2, (STATUS_H - 8) / 2);
    spr->print(buf);

    // Time remaining — right
    uint32_t secs = Engine.secondsRemaining();
    if (secs >= 60) snprintf(buf, sizeof(buf), "%dm %02ds", secs / 60, secs % 60);
    else            snprintf(buf, sizeof(buf), "%ds", secs);
    int tw2 = strlen(buf) * 6;
    spr->setCursor(SCREEN_W - tw2 - 6, (STATUS_H - 8) / 2);
    spr->print(buf);
}

// ── Word + ORP ─────────────────────────────────────────────────────────────
void Reader::drawWord(uint32_t index) {
    if (index >= Buffer.count()) return;
    const Word& w = Buffer.word(index);

    LGFX_Sprite* spr = Display.sprite();
    spr->fillRect(0, READING_Y, SCREEN_W, READING_H, COLOR_BG);

    // Split at ORP index
    uint8_t orp = w.orpIndex;
    String prefix  = w.text.substring(0, orp);
    String orpChar = w.text.substring(orp, orp + 1);
    String suffix  = w.text.substring(orp + 1);

    // Use font size 4 (≈26px tall in Arduino GFX) for main word
    // and size 5 for the ORP letter to draw the eye
    spr->setTextSize(4);
    int32_t wPrefix  = spr->textWidth(prefix);
    int32_t wSuffix  = spr->textWidth(suffix);
    spr->setTextSize(5);
    int32_t wOrp    = spr->textWidth(orpChar);

    int32_t totalW  = wPrefix + wOrp + wSuffix;
    int32_t startX  = (SCREEN_W - totalW) / 2;
    int32_t centerY = READING_Y + (READING_H / 2) - 16; // approx vertical center

    // Draw prefix
    spr->setTextSize(4);
    spr->setTextColor(COLOR_TEXT, COLOR_BG);
    spr->setCursor(startX, centerY);
    spr->print(prefix);

    // Draw ORP letter
    spr->setTextSize(5);
    spr->setTextColor(COLOR_ORP, COLOR_BG);
    spr->setCursor(startX + wPrefix, centerY - 4); // slight upward nudge for size diff
    spr->print(orpChar);

    // Draw suffix
    spr->setTextSize(4);
    spr->setTextColor(COLOR_TEXT, COLOR_BG);
    spr->setCursor(startX + wPrefix + wOrp, centerY);
    spr->print(suffix);
}

// ── Progress bar ───────────────────────────────────────────────────────────
void Reader::drawProgressBar() {
    LGFX_Sprite* spr = Display.sprite();
    spr->fillRect(0, PROGRESS_Y, SCREEN_W, PROGRESS_H, COLOR_PROGRESS_BG);

    uint32_t filled = 0;
    if (Buffer.count() > 0)
        filled = (uint32_t)SCREEN_W * Engine.index() / Buffer.count();
    spr->fillRect(0, PROGRESS_Y, (int32_t)filled, PROGRESS_H, COLOR_PROGRESS);
}

// ── Control buttons ────────────────────────────────────────────────────────
void Reader::drawControls(bool playing) {
    LGFX_Sprite* spr = Display.sprite();
    int y = SCREEN_H - CONTROLS_H;
    spr->fillRect(0, y, SCREEN_W, CONTROLS_H, COLOR_BG);

    static const char* labels[] = { "|<<", "<", "||", ">", ">>|", "-", "+" };
    // Button 2 (play/pause) swaps label based on state
    const char* playLabel = playing ? "||" : ">";

    for (int i = 0; i < 7; i++) {
        int x = btnLeft(i);
        bool isPlay = (i == 2);
        uint16_t bg = isPlay ? (playing ? COLOR_BTN_ACT : COLOR_BTN) : COLOR_BTN;
        spr->fillRoundRect(x + 4, y + 8, BTN_W - 8, CONTROLS_H - 16, 6, bg);
        spr->setTextColor(COLOR_TEXT, bg);
        spr->setTextSize(2);
        const char* lbl = (i == 2) ? playLabel : labels[i];
        int tw = strlen(lbl) * 12;
        spr->setCursor(x + (BTN_W - tw) / 2, y + (CONTROLS_H - 16) / 2);
        spr->print(lbl);
    }
}

// ── Full render ─────────────────────────────────────────────────────────────
void Reader::render(bool playing) {
    Display.sprite()->fillScreen(COLOR_BG);
    drawStatusBar();
    drawWord(Engine.index() > 0 ? Engine.index() - 1 : 0);
    drawProgressBar();
    drawControls(playing);
    Display.push();
}

// ── Touch → command ─────────────────────────────────────────────────────────
ReaderCmd Reader::handleTouch(const TouchEvent& evt) {
    if (evt.gesture == TouchGesture::NONE) return ReaderCmd::NONE;

    int y = evt.y;
    int x = evt.x;

    // Swipe gestures map to seek regardless of position
    if (evt.gesture == TouchGesture::SWIPE_LEFT)        return ReaderCmd::SEEK_BACK;
    if (evt.gesture == TouchGesture::SWIPE_RIGHT)       return ReaderCmd::SEEK_FWD;
    if (evt.gesture == TouchGesture::LONG_PRESS_LEFT)   return ReaderCmd::SENTENCE_BACK;
    if (evt.gesture == TouchGesture::LONG_PRESS_RIGHT)  return ReaderCmd::SENTENCE_FWD;

    if (evt.gesture == TouchGesture::TAP) {
        // Controls bar?
        if (y >= SCREEN_H - CONTROLS_H) {
            int btn = x / BTN_W;
            switch (btn) {
                case 0: return ReaderCmd::SENTENCE_BACK;
                case 1: return ReaderCmd::SEEK_BACK;
                case 2: return ReaderCmd::TOGGLE_PLAY;
                case 3: return ReaderCmd::SEEK_FWD;
                case 4: return ReaderCmd::SENTENCE_FWD;
                case 5: return ReaderCmd::WPM_DOWN;
                case 6: return ReaderCmd::WPM_UP;
            }
        }
        // Tap anywhere in reading area → toggle play/pause
        if (y >= READING_Y && y < READING_Y + READING_H) {
            return ReaderCmd::TOGGLE_PLAY;
        }
    }
    return ReaderCmd::NONE;
}
