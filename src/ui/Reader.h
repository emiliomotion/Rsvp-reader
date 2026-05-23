#pragma once
#include "../display/DisplayManager.h"
#include "../rsvp/RSVPEngine.h"
#include "../rsvp/TextBuffer.h"
#include "../config.h"

enum class ReaderCmd {
    NONE,
    TOGGLE_PLAY,
    SEEK_BACK,
    SEEK_FWD,
    SENTENCE_BACK,
    SENTENCE_FWD,
    WPM_DOWN,
    WPM_UP
};

class Reader {
public:
    void      render(bool playing);
    ReaderCmd handleTouch(const TouchEvent& evt);

private:
    void drawStatusBar();
    void drawWord(uint32_t index);
    void drawProgressBar();
    void drawControls(bool playing);

    static int16_t measureText(Arduino_GFX* g, const String& s, uint8_t sz);
};

extern Reader ReaderUI;
