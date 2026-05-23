#pragma once
#include <Arduino.h>
#include "TextBuffer.h"
#include "../config.h"

class RSVPEngine {
public:
    void     begin(uint32_t wordCount);
    bool     tick();          // returns true when word index advanced

    void     setWpm(int wpm);
    int      wpm()       const { return _wpm; }
    uint32_t index()     const { return _index; }
    uint32_t wordCount() const { return _count; }

    void     seek(int delta);         // ±N words
    void     seekSentence(int dir);   // -1 = prev, +1 = next sentence

    uint8_t  progressPct() const;
    uint32_t secondsRemaining() const;

private:
    int      _wpm      = WPM_DEFAULT;
    uint32_t _index    = 0;
    uint32_t _count    = 0;
    uint32_t _lastMs   = 0;
    uint32_t _interval = 60000 / WPM_DEFAULT; // ms per word
};

extern RSVPEngine Engine;
