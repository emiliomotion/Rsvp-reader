#include "Reader.h"

Reader ReaderUI;

// Arduino GFX built-in font: each char is 6w×8h at size 1.
// measureText approximates pixel width without getTextBounds overhead.
int16_t Reader::measureText(Arduino_GFX* g, const String& s, uint8_t sz) {
    return (int16_t)(s.length() * 6 * sz);
}

// ── Status bar (top 22px) ──────────────────────────────────────────────────
void Reader::drawStatusBar() {
    Arduino_GFX* g = Display.gfx();
    g->fillRect(0, 0, SCREEN_W, STATUS_H, COLOR_STATUS_BG);
    g->setTextSize(2);

    // WPM — left
    char buf[24];
    snprintf(buf, sizeof(buf), "%d WPM", Engine.wpm());
    g->setTextColor(COLOR_STATUS_TEXT);
    g->setCursor(6, (STATUS_H - 16) / 2);
    g->print(buf);

    // Progress % — center
    uint8_t pct = Engine.progressPct();
    snprintf(buf, sizeof(buf), "%d%%", pct);
    int16_t tw = measureText(g, String(buf), 2);
    g->setCursor((SCREEN_W - tw) / 2, (STATUS_H - 16) / 2);
    g->print(buf);

    // Time remaining — right
    uint32_t secs = Engine.secondsRemaining();
    if (secs >= 60) snprintf(buf, sizeof(buf), "%dm%02ds", secs / 60, secs % 60);
    else            snprintf(buf, sizeof(buf), "%ds", secs);
    int16_t tw2 = measureText(g, String(buf), 2);
    g->setCursor(SCREEN_W - tw2 - 6, (STATUS_H - 16) / 2);
    g->print(buf);
}

// ── Word + ORP (middle 100px) ──────────────────────────────────────────────
void Reader::drawWord(uint32_t index) {
    if (index >= Buffer.count()) return;
    const Word& w = Buffer.word(index);

    Arduino_GFX* g = Display.gfx();
    g->fillRect(0, READING_Y, SCREEN_W, READING_H, COLOR_BG);

    // Split word at ORP index
    String prefix  = w.text.substring(0, w.orpIndex);
    String orpChar = w.text.substring(w.orpIndex, w.orpIndex + 1);
    String suffix  = w.text.substring(w.orpIndex + 1);

    // Size 7 = 42×56px per char for main word; size 8 = 48×64px for ORP letter
    const uint8_t SZ_WORD = 7;
    const uint8_t SZ_ORP  = 8;

    int16_t wPre  = measureText(g, prefix,  SZ_WORD);
    int16_t wOrp  = measureText(g, orpChar, SZ_ORP);
    int16_t wSuf  = measureText(g, suffix,  SZ_WORD);
    int16_t total = wPre + wOrp + wSuf;

    int16_t startX  = (SCREEN_W - total) / 2;
    int16_t centerY = READING_Y + (READING_H - 56) / 2;  // vertically center size-7 height

    // Prefix (white)
    if (prefix.length() > 0) {
        g->setTextSize(SZ_WORD);
        g->setTextColor(COLOR_TEXT);
        g->setCursor(startX, centerY);
        g->print(prefix);
    }

    // ORP letter (red, slightly taller)
    g->setTextSize(SZ_ORP);
    g->setTextColor(COLOR_ORP);
    g->setCursor(startX + wPre, centerY - 4);  // nudge up to align baselines
    g->print(orpChar);

    // Suffix (white)
    if (suffix.length() > 0) {
        g->setTextSize(SZ_WORD);
        g->setTextColor(COLOR_TEXT);
        g->setCursor(startX + wPre + wOrp, centerY);
        g->print(suffix);
    }
}

// ── Progress bar (4px strip) ───────────────────────────────────────────────
void Reader::drawProgressBar() {
    Arduino_GFX* g = Display.gfx();
    g->fillRect(0, PROGRESS_Y, SCREEN_W, PROGRESS_H, COLOR_PROGRESS_BG);
    if (Buffer.count() > 0) {
        int32_t filled = (int32_t)SCREEN_W * Engine.index() / Buffer.count();
        g->fillRect(0, PROGRESS_Y, filled, PROGRESS_H, COLOR_PROGRESS);
    }
}

// ── Controls (bottom 46px) ────────────────────────────────────────────────
// 7 buttons across 640px: |<< | < | ⏸/▶ | > | >>| | -WPM | +WPM
void Reader::drawControls(bool playing) {
    Arduino_GFX* g = Display.gfx();
    const int BTN_W = SCREEN_W / 7;  // ~91px each

    static const char* labels[] = { "|<<", "<", "  ", ">", ">>|", "-", "+" };

    g->fillRect(0, CONTROLS_Y, SCREEN_W, CONTROLS_H, COLOR_BG);

    for (int i = 0; i < 7; i++) {
        int x = i * BTN_W;
        bool isPlay = (i == 2);
        uint16_t bg = isPlay ? COLOR_BTN_ACTIVE : COLOR_BTN;
        g->fillRoundRect(x + 3, CONTROLS_Y + 4, BTN_W - 6, CONTROLS_H - 8, 5, bg);

        const char* lbl = labels[i];
        if (isPlay) lbl = playing ? "||" : " >";

        g->setTextSize(2);
        g->setTextColor(COLOR_TEXT);
        int16_t tw = measureText(g, String(lbl), 2);
        g->setCursor(x + (BTN_W - tw) / 2, CONTROLS_Y + (CONTROLS_H - 16) / 2);
        g->print(lbl);
    }
}

// ── Full frame render ──────────────────────────────────────────────────────
void Reader::render(bool playing) {
    drawStatusBar();
    drawWord(Engine.index());
    drawProgressBar();
    drawControls(playing);
}

// ── Touch → command ────────────────────────────────────────────────────────
ReaderCmd Reader::handleTouch(const TouchEvent& evt) {
    if (evt.gesture == TouchGesture::NONE) return ReaderCmd::NONE;

    if (evt.gesture == TouchGesture::SWIPE_LEFT)       return ReaderCmd::SEEK_BACK;
    if (evt.gesture == TouchGesture::SWIPE_RIGHT)      return ReaderCmd::SEEK_FWD;
    if (evt.gesture == TouchGesture::LONG_PRESS_LEFT)  return ReaderCmd::SENTENCE_BACK;
    if (evt.gesture == TouchGesture::LONG_PRESS_RIGHT) return ReaderCmd::SENTENCE_FWD;

    if (evt.gesture == TouchGesture::TAP) {
        // Controls zone?
        if (evt.y >= CONTROLS_Y) {
            int btn = evt.x / (SCREEN_W / 7);
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
        // Tap on reading area → toggle play/pause
        if (evt.y >= READING_Y && evt.y < PROGRESS_Y) {
            return ReaderCmd::TOGGLE_PLAY;
        }
    }
    return ReaderCmd::NONE;
}
