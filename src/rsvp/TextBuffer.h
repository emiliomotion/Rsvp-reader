#pragma once
#include <Arduino.h>
#include <vector>
#include "../config.h"

struct Word {
    String  text;         // display text (stripped of leading/trailing punct)
    bool    sentenceEnd;  // true when original token ended with . ! ?
    uint8_t orpIndex;     // byte index of ORP fixation letter
};

class TextBuffer {
public:
    bool load(const char* path);
    void clear();

    const Word& word(uint32_t index) const { return _words[index]; }
    uint32_t    count()             const { return _words.size(); }

private:
    std::vector<Word> _words;

    static bool isSentenceEnd(char c) { return c == '.' || c == '!' || c == '?'; }
    static String stripPunct(const String& token, bool& sentenceEnd);
    static uint8_t calcOrp(uint8_t len);
};

extern TextBuffer Buffer;
