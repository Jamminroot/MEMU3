#include "../headers/debug/debug_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../headers/debug/stb_image.h"

void print_bitmap_console(BITMAP &bitmap) {
    static constexpr int scale_x = 1;
    static constexpr int scale_y = 3;

    int width = bitmap.bmWidth;
    int height = bitmap.bmHeight;
    int bytesPerPixel = bitmap.bmBitsPixel / 8;
    std::cout << "Bytes per pixel: " << bytesPerPixel << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    if (bitmap.bmBits == nullptr) {
        std::cerr << "Could not get bitmap data - bitmap.bmBits is null." << std::endl;
        return;
    }
    unsigned char* pixels = static_cast<unsigned char*>(bitmap.bmBits);

    if (pixels == nullptr) {
        std::cerr << "Could not get bitmap data." << std::endl;
        return;
    }

    for (int y = 0; y < height; y += scale_y) { // Adjusted the loop increments by scale
        for (int x = 0; x < width; x += scale_x) {
            int pixelIndex = (y * width + x) * bytesPerPixel;
            int r = pixels[pixelIndex + 2];
            int g = pixels[pixelIndex + 1];
            int b = pixels[pixelIndex];
            print_pixel_console(r, g, b);
        }
        std::cout << '\n';
    }
}

bool dump_bitmap(HBITMAP &hBitmap, LPCTSTR lpszFileName) {
    HDC hDC;
    int iBits;
    WORD wBitCount;
    DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
    BITMAP Bitmap0;
    BITMAPFILEHEADER bmfHdr;
    BITMAPINFOHEADER bi;
    LPBITMAPINFOHEADER lpbi;
    HANDLE fh, hDib, hPal, hOldPal2 = nullptr;
    hDC = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else
        wBitCount = 24;
    GetObject(hBitmap, sizeof(Bitmap0), (LPSTR)&Bitmap0);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = Bitmap0.bmWidth;
    bi.biHeight = -Bitmap0.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = wBitCount;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    bi.biClrUsed = 256;
    dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8
                   * Bitmap0.bmHeight;
    hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    *lpbi = bi;

    hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal)
    {
        hDC = GetDC(nullptr);
        hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
        RealizePalette(hDC);
    }


    GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
                                                       + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

    if (hOldPal2)
    {
        SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
        RealizePalette(hDC);
        ReleaseDC(nullptr, hDC);
    }

    fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

    if (fh == INVALID_HANDLE_VALUE)
        return FALSE;

    bmfHdr.bfType = 0x4D42; // "BM"
    dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, nullptr);

    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, nullptr);
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    CloseHandle(fh);
    return TRUE;
}

void print_hbitmap_console(HBITMAP &hbitmap) {
    BITMAP bitmap;
    GetObject(hbitmap, sizeof(BITMAP), &bitmap);
    print_bitmap_console(bitmap);
}

void print_pixel_console(unsigned char r, unsigned char g, unsigned char b) {
    static std::string asciiChars = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";

    auto color = 0;

    int gray = static_cast<int>(0.2989 * r + 0.5870 * g + 0.1140 * b);
    size_t asciiIndex = gray * (asciiChars.size() - 1) / 255;

    std::cout << "\033[38;2;" << static_cast<int>(r) << ";" << static_cast<int>(g) << ";" << static_cast<int>(b) << "m";
    std::cout << asciiChars[asciiIndex];
    std::cout << "\033[0m";
}

bool debug_print_layers(const std::vector<std::vector<Coords>> &layers, HBITMAP &hBitmap) {
    return true;
}

bool load_image_offset_region(const std::string &filename, const Rect &offset_region, HBITMAP &bitmap) {
    int width, height, channels;
    unsigned char* img = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if(img == nullptr) {
        std::cerr << "Could not open or find the image\n";
        return false;
    }

    HBITMAP hBmpSource = CreateBitmap(width, height, 1, channels * 8, img);

    auto screenshot_center_x = width / 2;
    auto screenshot_center_y = height / 2;

    auto region = offset_region;

    region.left = screenshot_center_x + region.left;
    region.top = screenshot_center_y + region.top;

    if(region.left < 0 || region.top < 0 || region.left + region.width > width || region.top + region.height > height) {
        std::cerr << "Invalid region\n";
        stbi_image_free(img);
        return false;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = region.width;
    bmi.bmiHeader.biHeight = -region.height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = channels * 8;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits;
    HBITMAP hBmpDest = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    if(!hBmpDest) {
        std::cerr << "Could not create destination bitmap\n";
        DeleteObject(hBmpSource);
        stbi_image_free(img);
        return false;
    }
    HDC hdc = CreateCompatibleDC(nullptr);
    SelectObject(hdc, hBmpDest);
    for(int y = region.top; y < region.height + region.top; ++y) {
        for(int x = region.left; x < region.left + region.width; ++x) {
            unsigned char* pixel = img + (y * width + x) * channels;
            auto r = pixel[0];
            auto g = pixel[1];
            auto b = pixel[2];
            SetPixel(hdc, x - region.left, y - region.top, RGB(r, g, b));
        }
    }
    bitmap = hBmpDest;
    stbi_image_free(img);
    DeleteDC(hdc);
    DeleteObject(hBmpSource);
    return true;
}
