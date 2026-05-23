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

    // Button X regions (7 buttons across 480px)
    // |<< | < | ⏸/▶ | > | >>| | -WPM | +WPM
    static const int BTN_W = 480 / 7;
    int btnLeft(int idx) const { return idx * BTN_W; }
};

extern Reader ReaderUI;
