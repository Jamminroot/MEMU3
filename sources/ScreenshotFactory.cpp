#include "../headers/ScreenshotFactory.h"
#include "../headers/Utils.h"
#include "../headers/logging/logger.h"
#include <Windows.h>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

ScreenshotFactory::ScreenshotFactory(const struct Rect &coords) : region(coords) {
    sourceDC = GetDC(nullptr);
    hbitmap = CreateCompatibleBitmap(sourceDC, region.width, region.height);
    targetDC = CreateCompatibleDC(sourceDC);
}

bool ScreenshotFactory::update_screenshot_data(ScreenshotData &screenshot) {
    if (!refresh_capture()){
        Logger::show("Failed to refresh capture");
        return false;
    }
    if (!hbitmap) {
        Logger::show("ERROR: Bitmap creation failed!");
        return false;
    }

    return update_screenshot_from_region_bitmap(screenshot, hbitmap);
}

bool ScreenshotFactory::update_screenshot_from_region_bitmap(ScreenshotData &screenshot, HBITMAP &p_hbitmap) {
    SelectObject(sourceDC, p_hbitmap);

    // Perform BitBlt
    if (!BitBlt(targetDC, 0, 0, region.width, region.height, sourceDC, region.left, region.top, SRCCOPY | CAPTUREBLT)) {
        Logger::show("ERROR: bit-block transfer failed!");
        return false;
    }

    // Get Bitmap Info
    BITMAPINFO bmpInfo = create_bitmap_info_struct(region.width, region.height, 32);
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    if (!GetDIBits(targetDC, p_hbitmap, 0, 0, nullptr, &bmpInfo, DIB_RGB_COLORS)) {
        Logger::show("ERROR: Failed to get Bitmap Info.");
        return false;
    }

    bmpInfo.bmiHeader.biCompression = BI_RGB;

    // Get the Pixel Data
    if (!GetDIBits(targetDC, p_hbitmap, 0, bmpInfo.bmiHeader.biHeight, (LPVOID)screenshot.data, &bmpInfo, DIB_RGB_COLORS)) {
        Logger::show( "ERROR: Getting the bitmap buffer!");
        return false;
    }

    return true;
}

bool ScreenshotFactory::refresh_capture() {
    SelectObject(targetDC, hbitmap);

    if (!BitBlt(targetDC, 0, 0, region.width, region.height, sourceDC, region.left, region.top, SRCCOPY | CAPTUREBLT)) {
        Logger::show("ERROR: bit-block transfer failed!");
        return false;
    }
    return true;
}

void ScreenshotFactory::release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp) {
    DeleteObject(hBmp);
    DeleteDC(captureDC);
    ReleaseDC(nullptr, hdc);
}

const Rect &ScreenshotFactory::get_region() const { return region; }

ScreenshotFactory::~ScreenshotFactory() {
    release(sourceDC, targetDC, hbitmap);
}
