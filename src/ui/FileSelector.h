#pragma once
#include <Arduino.h>
#include <vector>
#include "../display/DisplayManager.h"

class FileSelector {
public:
    void   begin();           // scan SD root for .txt files
    void   render();          // draw list to display
    String handleTouch(const TouchEvent& evt); // returns path or ""

private:
    std::vector<String> _files;
    int _scrollOffset = 0;

    // Rows: 640×172 in landscape — use full height, wide rows
    static const int ROW_H = 38;
    static const int TITLE_H = 28;
    static const int ROWS = (SCREEN_H - TITLE_H) / ROW_H;  // 3 rows visible

    void drawRow(int screenRow, const String& name);
};

extern FileSelector FileSel;
