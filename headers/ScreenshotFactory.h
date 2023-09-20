#pragma once

#include "ScreenshotData.h"

class ScreenshotFactory {
public:
    explicit ScreenshotFactory(Rect &rect);
    bool update_screenshot_data(ScreenshotData &screenshot);
    static bool update_screenshot_from_region_bitmap(ScreenshotData &screenshot, HBITMAP &hBmp);
    const Rect &get_region() const;
private:
    Rect region;
    HBITMAP capture_region();
    static void release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp);
};

