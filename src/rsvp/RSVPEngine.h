#pragma once
#include <Arduino.h>
#include "TextBuffer.h"
#include "../config.h"

class RSVPEngine {
public:
    void     begin(uint32_t wordCount);

    // Returns true when the current word's display interval has elapsed.
    // Does NOT advance the index — call advance() after.
    bool     tick();

    // Advance to the next word and reset the interval timer.
    void     advance();

    void     setWpm(int wpm);
    int      wpm()       const { return _wpm; }
    uint32_t index()     const { return _index; }
    uint32_t wordCount() const { return _count; }

    void     seek(int delta);          // ±N words, resets timer
    void     seekSentence(int dir);    // -1 = prev sentence, +1 = next sentence

    uint8_t  progressPct()       const;
    uint32_t secondsRemaining()  const;

private:
    int      _wpm      = WPM_DEFAULT;
    uint32_t _index    = 0;
    uint32_t _count    = 0;
    uint32_t _lastMs   = 0;
    uint32_t _interval = 60000 / WPM_DEFAULT;
};

extern RSVPEngine Engine;
