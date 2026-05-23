#pragma once
#include <Arduino.h>
#include <vector>
#include "../display/DisplayManager.h"

class FileSelector {
public:
    void     begin();          // scan SD for .txt files
    void     render();         // draw current list page to sprite
    String   handleTouch(const TouchEvent& evt); // returns path or ""

private:
    std::vector<String> _files;
    int _scrollOffset = 0;    // first visible row index

    static const int ROW_H  = 44;
    static const int ROWS   = SCREEN_H / ROW_H;  // visible rows

    void drawRow(int row, const String& name, bool highlighted);
};

extern FileSelector FileSel;
