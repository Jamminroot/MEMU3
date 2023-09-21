#include "../headers/ScreenshotFactory.h"
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

ScreenshotFactory::ScreenshotFactory(class Rect &coords) : region(coords) { }

bool ScreenshotFactory::update_screenshot_data(ScreenshotData &screenshot) {
    HBITMAP hBmp = capture_region();
    if (!hBmp) {
        cout << "ERROR: Bitmap creation failed!" << endl;
        return false;
    }

    return update_screenshot_from_region_bitmap(screenshot, hBmp);
}

bool ScreenshotFactory::update_screenshot_from_region_bitmap(ScreenshotData &screenshot, HBITMAP &hBmp) {
    HDC hdc = GetDC(nullptr);
    HDC captureDC = CreateCompatibleDC(hdc);
    SelectObject(captureDC, hBmp);

    BITMAP bitmap;
    GetObject(hBmp, sizeof(BITMAP), &bitmap);

    BITMAPINFO bmpInfo = {0};
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = bitmap.bmWidth;
    bmpInfo.bmiHeader.biHeight = bitmap.bmHeight;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    GetDIBits(hdc, hBmp, 0, bitmap.bmHeight, (LPVOID) screenshot.data, &bmpInfo, DIB_RGB_COLORS);
    if (GetLastError() != ERROR_SUCCESS) {
        cerr << "ERROR: Getting the bitmap buffer!" << endl;
        release(hdc, captureDC, hBmp);
        return false;
    }

    DeleteDC(hdc);
    DeleteDC(captureDC);

    return true;
}

HBITMAP ScreenshotFactory::capture_region() {
    HDC hdc = GetDC(nullptr);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, region.width, region.height);

    if (!hBmp) {
        release(hdc, hdc, hBmp);
        return nullptr;
    }

    HDC captureDC = CreateCompatibleDC(hdc);
    SelectObject(captureDC, hBmp);

    if (!BitBlt(captureDC, 0, 0, region.width, region.height, hdc, region.left, region.top, SRCCOPY | CAPTUREBLT)) {
        cout << "ERROR: bit-block transfer failed!" << endl;
        release(hdc, captureDC, hBmp);
        return nullptr;
    }

    release(hdc, captureDC, hBmp);
    return hBmp;
}

void ScreenshotFactory::release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp) {
    DeleteObject(hBmp);
    DeleteDC(captureDC);
    ReleaseDC(nullptr, hdc);
}

const Rect &ScreenshotFactory::get_region() const { return region; }
