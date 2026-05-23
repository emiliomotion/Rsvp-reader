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

void FileSelector::drawRow(int screenRow, const String& path) {
    Arduino_GFX* g = Display.gfx();
    int y = TITLE_H + screenRow * ROW_H;

    g->fillRect(2, y + 1, SCREEN_W - 4, ROW_H - 2, COLOR_BTN);

    // Strip leading slash + .txt for display
    String label = path;
    if (label.startsWith("/")) label = label.substring(1);
    if (label.endsWith(".txt"))  label = label.substring(0, label.length() - 4);
    if (label.length() > 50)     label = label.substring(0, 48) + "..";

    g->setTextSize(2);
    g->setTextColor(COLOR_TEXT);
    g->setCursor(12, y + (ROW_H - 16) / 2);
    g->print(label);
}

void FileSelector::render() {
    Arduino_GFX* g = Display.gfx();
    g->fillScreen(COLOR_BG);

    // Title bar
    g->fillRect(0, 0, SCREEN_W, TITLE_H, COLOR_STATUS_BG);
    g->setTextSize(2);
    g->setTextColor(COLOR_STATUS_TEXT);
    g->setCursor(8, (TITLE_H - 16) / 2);
    g->print("Select a file  (");
    g->print((int)_files.size());
    g->print(" found)");

    if (_files.empty()) {
        g->setTextColor(COLOR_STATUS_TEXT);
        g->setTextSize(2);
        g->setCursor(120, SCREEN_H / 2 - 8);
        g->print("No .txt files on SD card");
        return;
    }

    int visEnd = min((int)_files.size(), _scrollOffset + ROWS);
    for (int i = _scrollOffset; i < visEnd; i++) {
        drawRow(i - _scrollOffset, _files[i]);
    }

    // Scroll indicator on right edge
    if ((int)_files.size() > ROWS) {
        int barH = (SCREEN_H - TITLE_H) * ROWS / (int)_files.size();
        int barY = TITLE_H + (_scrollOffset * (SCREEN_H - TITLE_H)) / (int)_files.size();
        g->fillRect(SCREEN_W - 5, barY, 5, barH, COLOR_PROGRESS);
    }
}

String FileSelector::handleTouch(const TouchEvent& evt) {
    if (evt.gesture == TouchGesture::SWIPE_LEFT) {
        _scrollOffset = min(_scrollOffset + 1, (int)_files.size() - 1);
    } else if (evt.gesture == TouchGesture::SWIPE_RIGHT) {
        _scrollOffset = max(0, _scrollOffset - 1);
    } else if (evt.gesture == TouchGesture::TAP && evt.y >= TITLE_H) {
        int row = (evt.y - TITLE_H) / ROW_H + _scrollOffset;
        if (row >= 0 && row < (int)_files.size()) {
            return _files[row];
        }
    }
    return "";
}
