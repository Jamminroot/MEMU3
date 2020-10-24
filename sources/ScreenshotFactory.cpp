#include "../headers/ScreenshotFactory.h"
#include <Windows.h>
#include <WinUser.h>
#include <Ole2.h>
#include <OleCtl.h>
#include <iostream>

using namespace std;

ScreenshotFactory::ScreenshotFactory(class Manager &pManager) : manager(pManager) {
}

bool saveBitmap(const wchar_t *filename, HBITMAP bmp) {
    bool result = false;
    HPALETTE pal = nullptr;
    PICTDESC pd;

    pd.cbSizeofstruct = sizeof(PICTDESC);
    pd.picType = PICTYPE_BITMAP;
    pd.bmp.hbitmap = bmp;
    pd.bmp.hpal = pal;

    LPPICTURE picture;
    HRESULT res = OleCreatePictureIndirect(&pd, IID_IPicture, false, reinterpret_cast<void **>(&picture));

    if (!SUCCEEDED(res))
        return false;

    LPSTREAM stream;
    res = CreateStreamOnHGlobal(nullptr, true, &stream);

    if (!SUCCEEDED(res)) {
        picture->Release();
        return false;
    }

    LONG bytes_streamed;
    res = picture->SaveAsFile(stream, true, &bytes_streamed);

    HANDLE file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (!SUCCEEDED(res) || !file) {
        stream->Release();
        picture->Release();
        return false;
    }

    HGLOBAL mem = 0;
    GetHGlobalFromStream(stream, &mem);
    LPVOID data = GlobalLock(mem);

    DWORD bytes_written;

    result = WriteFile(file, data, bytes_streamed, &bytes_written, 0);
    result &= (bytes_written == static_cast<DWORD>(bytes_streamed));

    GlobalUnlock(mem);
    CloseHandle(file);

    stream->Release();
    picture->Release();

    return result;
}

bool ScreenshotFactory::update_screenshot() {

    HDC hdc = GetDC(nullptr);
    HDC captureDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, manager.region.width, manager.region.height);
    HGDIOBJ hOld = SelectObject(captureDC, hBmp);

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


    //if (saveBitmap(L"c:\\temp\\dbg.bmp", hBmp)) return true;
    release(hdc, captureDC, hBmp);

    return true;
}

void ScreenshotFactory::release(HDC &hdc, HDC &captureDC, HBITMAP &hBmp) {
    DeleteObject(hBmp);
    DeleteDC(captureDC);
    DeleteDC(hdc);
}


