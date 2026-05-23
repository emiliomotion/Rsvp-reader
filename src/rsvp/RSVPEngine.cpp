#include "RSVPEngine.h"

RSVPEngine Engine;

void RSVPEngine::begin(uint32_t wordCount) {
    _count    = wordCount;
    _index    = 0;
    _lastMs   = 0;
    _wpm      = WPM_DEFAULT;
    _interval = 60000 / _wpm;
}

bool RSVPEngine::tick() {
    if (_index >= _count) return false;
    uint32_t now = millis();
    if ((uint32_t)(now - _lastMs) >= _interval) {
        _lastMs = now;
        if (_index < _count) _index++;
        return true;
    }
    return false;
}

void RSVPEngine::setWpm(int wpm) {
    _wpm = constrain(wpm, WPM_MIN, WPM_MAX);
    _interval = 60000 / _wpm;
}

void RSVPEngine::seek(int delta) {
    int next = (int)_index + delta;
    if (next < 0) next = 0;
    if ((uint32_t)next > _count) next = (int)_count;
    _index  = (uint32_t)next;
    _lastMs = millis(); // reset timing from new position
}

void RSVPEngine::seekSentence(int dir) {
    if (dir > 0) {
        // Advance to the word after the next sentence boundary
        uint32_t i = _index;
        while (i < _count) {
            if (Buffer.word(i).sentenceEnd) { i++; break; }
            i++;
        }
        _index = i < _count ? i : _count;
    } else {
        // Step back past current position's sentence boundary, then to start of that sentence
        int i = (int)_index - 1;
        // Skip over a boundary immediately behind us if we're at one
        if (i > 0 && Buffer.word(i).sentenceEnd) i--;
        while (i > 0 && !Buffer.word(i - 1).sentenceEnd) i--;
        _index = (uint32_t)(i < 0 ? 0 : i);
    }
    _lastMs = millis();
}

uint8_t RSVPEngine::progressPct() const {
    if (_count == 0) return 0;
    return (uint8_t)((_index * 100UL) / _count);
}

uint32_t RSVPEngine::secondsRemaining() const {
    if (_count == 0 || _index >= _count) return 0;
    uint32_t wordsLeft = _count - _index;
    return (wordsLeft * 60UL) / (uint32_t)_wpm;
}
