#pragma once

#include <Windows.h>
#include "Rect.h"

class ScreenshotData {
public:
    ScreenshotData();
    ScreenshotData(const Rect &regionSize);
    ~ScreenshotData();
    RGBQUAD data[600 * 500]{0};
    int size;
};