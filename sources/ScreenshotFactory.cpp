#include "../headers/ScreenshotFactory.h"
#include <Windows.h>
#include <WinUser.h>
#include <iostream>

#pragma comment(lib, "Gdi32.lib")

using namespace std;

ScreenshotFactory::ScreenshotFactory(class Manager &pManager) : manager(pManager) { }
bool ScreenshotFactory::update_screenshot() {

    HDC hdc = GetDC(nullptr);
    HDC captureDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, manager.region.width, manager.region.height);
    SelectObject(captureDC, hBmp);

    if (!BitBlt(captureDC, 0, 0, manager.region.width, manager.region.height, hdc, manager.region.left, manager.region.top, SRCCOPY | CAPTUREBLT)) {
        cout << "ERROR: bit-block transfer failed!" << endl;
        release(hdc, captureDC, hBmp);
        return false;
    }

    SelectObject(captureDC, hBmp);

    BITMAPINFO bmpInfo = {0};
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    if (!GetDIBits(hdc, hBmp, 0, 0, nullptr, &bmpInfo, DIB_RGB_COLORS)) //get bmpInfo
    {
        cout << "ERROR: Failed to get Bitmap Info." << endl;
        release(hdc, captureDC, hBmp);
        return false;
    }

    bmpInfo.bmiHeader.biCompression = BI_RGB;

    if (!GetDIBits(hdc, hBmp, 0, bmpInfo.bmiHeader.biHeight, (LPVOID) manager.screenshot.data, &bmpInfo, DIB_RGB_COLORS)) {
        cout << "ERROR: Getting the bitmap buffer!" << endl;
        release(hdc, captureDC, hBmp);
        return false;
    }

    release(hdc, captureDC, hBmp);

    return true;
}

void ScreenshotFactory::release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp) {
    DeleteObject(hBmp);
    DeleteDC(captureDC);
    DeleteDC(hdc);
}


