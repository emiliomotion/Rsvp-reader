#include "TextBuffer.h"
#include <SD_MMC.h>

TextBuffer Buffer;

static const size_t CHUNK = 512;

String TextBuffer::stripPunct(const String& token, bool& sentenceEnd) {
    int start = 0;
    int end   = (int)token.length() - 1;

    // Check for sentence-ending punctuation before stripping
    sentenceEnd = (end >= 0) && isSentenceEnd(token[end]);

    // Strip leading non-alpha-numeric
    while (start <= end && !isalnum((unsigned char)token[start])) start++;
    // Strip trailing non-alpha-numeric
    while (end >= start && !isalnum((unsigned char)token[end])) end--;

    if (end < start) return token; // nothing to strip, return original
    return token.substring(start, end + 1);
}

uint8_t TextBuffer::calcOrp(uint8_t len) {
    if (len <= 1) return 0;
    uint8_t idx = (uint8_t)(len * ORP_RATIO);
    return idx < len ? idx : len - 1;
}

bool TextBuffer::load(const char* path) {
    clear();

    File f = SD_MMC.open(path, FILE_READ);
    if (!f) return false;

    String token;
    char   buf[CHUNK];

    while (f.available()) {
        int n = f.read((uint8_t*)buf, sizeof(buf));
        for (int i = 0; i < n; i++) {
            char c = buf[i];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                if (token.length() > 0) {
                    bool sentenceEnd = false;
                    String display = stripPunct(token, sentenceEnd);
                    if (display.length() > 0) {
                        Word w;
                        w.text        = display;
                        w.sentenceEnd = sentenceEnd;
                        w.orpIndex    = calcOrp((uint8_t)min((int)display.length(), 255));
                        _words.push_back(w);
                    }
                    token = "";
                }
            } else {
                token += c;
            }
        }
    }
    // Last token
    if (token.length() > 0) {
        bool sentenceEnd = false;
        String display = stripPunct(token, sentenceEnd);
        if (display.length() > 0) {
            Word w;
            w.text        = display;
            w.sentenceEnd = sentenceEnd;
            w.orpIndex    = calcOrp((uint8_t)min((int)display.length(), 255));
            _words.push_back(w);
        }
    }

    f.close();
    return _words.size() > 0;
}

void TextBuffer::clear() {
    _words.clear();
    _words.shrink_to_fit();
}
