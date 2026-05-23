#include <Arduino.h>
#include <SD_MMC.h>
#include "config.h"
#include "display/DisplayManager.h"
#include "rsvp/TextBuffer.h"
#include "rsvp/RSVPEngine.h"
#include "ui/FileSelector.h"
#include "ui/Reader.h"

enum class AppState : uint8_t { BROWSE, PAUSED, READING };

static AppState state      = AppState::BROWSE;
static bool     needRender = true;

static void showMessage(const char* msg) {
    Arduino_GFX* g = Display.gfx();
    g->fillScreen(COLOR_BG);
    g->setTextColor(COLOR_STATUS_TEXT);
    g->setTextSize(2);
    g->setCursor(20, SCREEN_H / 2 - 8);
    g->print(msg);
}

static bool initSD() {
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_DATA0);
    return SD_MMC.begin("/sdcard", /*mode1bit=*/true);
}

void setup() {
    Serial.begin(115200);

    if (!Display.begin()) {
        Serial.println("Display init failed");
        while (true) delay(100);
    }

    if (!initSD()) {
        showMessage("SD card not found!");
        Serial.println("SD_MMC init failed");
        while (true) delay(500);
    }

    FileSel.begin();
    needRender = true;
}

void loop() {
    TouchEvent evt = Display.pollTouch();

    switch (state) {

        case AppState::BROWSE: {
            String chosen = FileSel.handleTouch(evt);
            if (chosen.length() > 0) {
                showMessage("Loading...");
                if (Buffer.load(chosen.c_str()) && Buffer.count() > 0) {
                    Engine.begin(Buffer.count());
                    state = AppState::PAUSED;
                    needRender = true;
                    Serial.printf("Loaded %u words\n", Buffer.count());
                } else {
                    showMessage("Failed to load file");
                    delay(1500);
                    FileSel.begin();
                    needRender = true;
                }
            } else if (needRender || evt.gesture != TouchGesture::NONE) {
                FileSel.render();
                needRender = false;
            }
            break;
        }

        case AppState::PAUSED: {
            ReaderCmd cmd = ReaderUI.handleTouch(evt);
            bool changed = (cmd != ReaderCmd::NONE);

            switch (cmd) {
                case ReaderCmd::TOGGLE_PLAY:   state = AppState::READING; break;
                case ReaderCmd::SEEK_BACK:     Engine.seek(-SEEK_WORDS);  break;
                case ReaderCmd::SEEK_FWD:      Engine.seek(+SEEK_WORDS);  break;
                case ReaderCmd::SENTENCE_BACK: Engine.seekSentence(-1);   break;
                case ReaderCmd::SENTENCE_FWD:  Engine.seekSentence(+1);   break;
                case ReaderCmd::WPM_DOWN:      Engine.setWpm(Engine.wpm() - WPM_STEP); break;
                case ReaderCmd::WPM_UP:        Engine.setWpm(Engine.wpm() + WPM_STEP); break;
                default: break;
            }

            if (needRender || changed) {
                ReaderUI.render(false);
                needRender = false;
            }
            break;
        }

        case AppState::READING: {
            ReaderCmd cmd = ReaderUI.handleTouch(evt);
            bool changed = (cmd != ReaderCmd::NONE);

            switch (cmd) {
                case ReaderCmd::TOGGLE_PLAY:   state = AppState::PAUSED;  changed = true; break;
                case ReaderCmd::SEEK_BACK:     Engine.seek(-SEEK_WORDS);  break;
                case ReaderCmd::SEEK_FWD:      Engine.seek(+SEEK_WORDS);  break;
                case ReaderCmd::SENTENCE_BACK: Engine.seekSentence(-1);   break;
                case ReaderCmd::SENTENCE_FWD:  Engine.seekSentence(+1);   break;
                case ReaderCmd::WPM_DOWN:      Engine.setWpm(Engine.wpm() - WPM_STEP); break;
                case ReaderCmd::WPM_UP:        Engine.setWpm(Engine.wpm() + WPM_STEP); break;
                default: break;
            }

            if (Engine.index() >= Engine.wordCount()) {
                state = AppState::PAUSED;
                changed = true;
            } else if (Engine.tick()) {
                Engine.advance();
                changed = true;
            }

            if (needRender || changed) {
                ReaderUI.render(state == AppState::READING);
                needRender = false;
            }
            break;
        }
    }
}
