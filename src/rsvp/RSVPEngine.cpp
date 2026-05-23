#include "RSVPEngine.h"

RSVPEngine Engine;

void RSVPEngine::begin(uint32_t wordCount) {
    _count    = wordCount;
    _index    = 0;
    _lastMs   = millis();
    _wpm      = WPM_DEFAULT;
    _interval = 60000 / _wpm;
}

bool RSVPEngine::tick() {
    if (_index >= _count) return false;
    return (uint32_t)(millis() - _lastMs) >= _interval;
}

void RSVPEngine::advance() {
    if (_index < _count) _index++;
    _lastMs = millis();
}

void RSVPEngine::setWpm(int wpm) {
    _wpm = constrain(wpm, WPM_MIN, WPM_MAX);
    _interval = 60000 / _wpm;
}

void RSVPEngine::seek(int delta) {
    int next = (int)_index + delta;
    _index  = (uint32_t)constrain(next, 0, (int)_count);
    _lastMs = millis();
}

void RSVPEngine::seekSentence(int dir) {
    if (dir > 0) {
        uint32_t i = _index;
        while (i < _count) {
            if (Buffer.word(i).sentenceEnd) { i++; break; }
            i++;
        }
        _index = i < _count ? i : _count;
    } else {
        int i = (int)_index - 1;
        if (i > 0 && Buffer.word(i).sentenceEnd) i--;
        while (i > 0 && !Buffer.word(i - 1).sentenceEnd) i--;
        _index = (uint32_t)max(i, 0);
    }
    _lastMs = millis();
}

uint8_t RSVPEngine::progressPct() const {
    if (_count == 0) return 0;
    return (uint8_t)((_index * 100UL) / _count);
}

uint32_t RSVPEngine::secondsRemaining() const {
    if (_count == 0 || _index >= _count) return 0;
    return ((_count - _index) * 60UL) / (uint32_t)_wpm;
}
