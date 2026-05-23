#include <Arduino.h>
#include <SD_MMC.h>
#include "config.h"
#include "display/DisplayManager.h"
#include "rsvp/TextBuffer.h"
#include "rsvp/RSVPEngine.h"
#include "ui/FileSelector.h"
#include "ui/Reader.h"

enum class AppState : uint8_t { BROWSE, PAUSED, READING };

static AppState state = AppState::BROWSE;

static void showLoading(const String& path) {
    LGFX_Sprite* spr = Display.sprite();
    spr->fillScreen(COLOR_BG);
    spr->setTextColor(COLOR_TEXT, COLOR_BG);
    spr->setTextSize(2);
    spr->setCursor(20, SCREEN_H / 2 - 8);
    spr->print("Loading...");
    Display.push();
}

static bool initSD() {
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_DATA0);
    return SD_MMC.begin("/sdcard", true); // 1-bit mode
}

void setup() {
    Serial.begin(115200);

    if (!Display.begin()) {
        Serial.println("Display init failed");
        while (true) delay(100);
    }

    if (!initSD()) {
        LGFX_Sprite* spr = Display.sprite();
        spr->fillScreen(COLOR_BG);
        spr->setTextColor(COLOR_ORP, COLOR_BG);
        spr->setTextSize(2);
        spr->setCursor(20, SCREEN_H / 2 - 8);
        spr->print("SD card not found!");
        Display.push();
        Serial.println("SD_MMC init failed");
        while (true) delay(500);
    }

    FileSel.begin();
    state = AppState::BROWSE;
}

void loop() {
    TouchEvent evt = Display.pollTouch();

    switch (state) {
        case AppState::BROWSE: {
            FileSel.render();
            String chosen = FileSel.handleTouch(evt);
            if (chosen.length() > 0) {
                showLoading(chosen);
                if (Buffer.load(chosen.c_str()) && Buffer.count() > 0) {
                    Engine.begin(Buffer.count());
                    state = AppState::PAUSED;
                    Serial.printf("Loaded %u words from %s\n", Buffer.count(), chosen.c_str());
                } else {
                    Serial.println("Failed to load file or empty");
                }
            }
            break;
        }

        case AppState::PAUSED: {
            ReaderCmd cmd = ReaderUI.handleTouch(evt);
            switch (cmd) {
                case ReaderCmd::TOGGLE_PLAY:    state = AppState::READING; break;
                case ReaderCmd::SEEK_BACK:      Engine.seek(-SEEK_WORDS);  break;
                case ReaderCmd::SEEK_FWD:       Engine.seek(+SEEK_WORDS);  break;
                case ReaderCmd::SENTENCE_BACK:  Engine.seekSentence(-1);   break;
                case ReaderCmd::SENTENCE_FWD:   Engine.seekSentence(+1);   break;
                case ReaderCmd::WPM_DOWN:       Engine.setWpm(Engine.wpm() - WPM_STEP); break;
                case ReaderCmd::WPM_UP:         Engine.setWpm(Engine.wpm() + WPM_STEP); break;
                default: break;
            }
            ReaderUI.render(false);
            break;
        }

        case AppState::READING: {
            ReaderCmd cmd = ReaderUI.handleTouch(evt);
            switch (cmd) {
                case ReaderCmd::TOGGLE_PLAY:    state = AppState::PAUSED;  break;
                case ReaderCmd::SEEK_BACK:      Engine.seek(-SEEK_WORDS);  break;
                case ReaderCmd::SEEK_FWD:       Engine.seek(+SEEK_WORDS);  break;
                case ReaderCmd::SENTENCE_BACK:  Engine.seekSentence(-1);   break;
                case ReaderCmd::SENTENCE_FWD:   Engine.seekSentence(+1);   break;
                case ReaderCmd::WPM_DOWN:       Engine.setWpm(Engine.wpm() - WPM_STEP); break;
                case ReaderCmd::WPM_UP:         Engine.setWpm(Engine.wpm() + WPM_STEP); break;
                default: break;
            }

            if (Engine.index() >= Engine.wordCount()) {
                state = AppState::PAUSED; // reached end
            } else {
                Engine.tick();
            }

            ReaderUI.render(true);
            break;
        }
    }
}
