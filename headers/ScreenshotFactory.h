#pragma once

#include "ScreenshotData.h"

class ScreenshotFactory {
public:
    explicit ScreenshotFactory(const Rect &rect);
    ~ScreenshotFactory();
    bool update_screenshot_data(ScreenshotData &screenshot);
    bool update_screenshot_from_region_bitmap(ScreenshotData &screenshot, HBITMAP &hBmp);
    const Rect &get_region() const;
private:
    HDC sourceDC;
    HDC targetDC;
    HBITMAP hbitmap;
    Rect region;
    bool refresh_capture();
    static void release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp);
};

