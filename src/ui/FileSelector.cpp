#include "FileSelector.h"
#include <SD_MMC.h>

FileSelector FileSel;

void FileSelector::begin() {
    _files.clear();
    _scrollOffset = 0;

    File root = SD_MMC.open("/");
    if (!root) return;

    File f = root.openNextFile();
    while (f) {
        if (!f.isDirectory()) {
            String name = String(f.name());
            if (name.endsWith(".txt") || name.endsWith(".TXT")) {
                _files.push_back("/" + name);
            }
        }
        f = root.openNextFile();
    }
    root.close();
}

void FileSelector::drawRow(int row, const String& name, bool highlighted) {
    LGFX_Sprite* spr = Display.sprite();
    int y = row * ROW_H;

    uint16_t bg = highlighted ? COLOR_BTN_ACT : COLOR_BTN;
    spr->fillRect(0, y, SCREEN_W, ROW_H - 2, bg);

    spr->setTextColor(COLOR_TEXT, bg);
    spr->setTextSize(2);
    spr->setCursor(12, y + (ROW_H - 16) / 2);

    // Strip leading slash and .txt extension for display
    String display = name;
    if (display.startsWith("/")) display = display.substring(1);
    if (display.endsWith(".txt")) display = display.substring(0, display.length() - 4);
    if (display.length() > 30) display = display.substring(0, 28) + "..";

    spr->print(display);
}

void FileSelector::render() {
    LGFX_Sprite* spr = Display.sprite();
    spr->fillScreen(COLOR_BG);

    // Title bar
    spr->fillRect(0, 0, SCREEN_W, 28, COLOR_STATUS);
    spr->setTextColor(COLOR_BG, COLOR_STATUS);
    spr->setTextSize(1);
    spr->setCursor(8, 8);
    spr->print("Select a file");

    if (_files.empty()) {
        spr->setTextColor(COLOR_STATUS, COLOR_BG);
        spr->setTextSize(2);
        spr->setCursor(60, SCREEN_H / 2 - 8);
        spr->print("No .txt files on SD");
        Display.push();
        return;
    }

    int visibleStart = _scrollOffset;
    int visibleEnd   = min((int)_files.size(), visibleStart + ROWS);

    for (int i = visibleStart; i < visibleEnd; i++) {
        drawRow(i - visibleStart + (28 / ROW_H + 1), _files[i], false);
    }

    // Scroll indicator
    if ((int)_files.size() > ROWS) {
        int barH = SCREEN_H * ROWS / (int)_files.size();
        int barY = STATUS_H + (_scrollOffset * (SCREEN_H - STATUS_H)) / (int)_files.size();
        spr->fillRect(SCREEN_W - 6, barY, 6, barH, COLOR_PROGRESS);
    }

    Display.push();
}

String FileSelector::handleTouch(const TouchEvent& evt) {
    if (evt.gesture == TouchGesture::SWIPE_LEFT) {
        // Scroll down
        _scrollOffset = min(_scrollOffset + 1, (int)_files.size() - 1);
    } else if (evt.gesture == TouchGesture::SWIPE_RIGHT) {
        // Scroll up
        _scrollOffset = max(0, _scrollOffset - 1);
    } else if (evt.gesture == TouchGesture::TAP) {
        // Which row was tapped?
        int row = (evt.y - 28) / ROW_H + _scrollOffset;
        if (row >= 0 && row < (int)_files.size()) {
            return _files[row];
        }
    }
    return "";
}
