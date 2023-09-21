#include "../headers/debug/debug_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../headers/debug/stb_image.h"
#include "../../headers/Utils.h"


void print_bitmap_console(BITMAP &bitmap, int scale_x, int scale_y) {

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

void print_hbitmap_console(HBITMAP &hbitmap, int scale_x, int scale_y) {
    BITMAP bitmap;
    GetObject(hbitmap, sizeof(BITMAP), &bitmap);
    print_bitmap_console(bitmap, scale_x, scale_y);
}

bool dump_bitmap(HBITMAP &hBitmap, const std::string &filename) {
    HDC hDC;
    int iBits;
    WORD wBitCount;
    DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
    BITMAP Bitmap0;
    BITMAPFILEHEADER bmfHdr;
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
    BITMAPINFOHEADER bi = create_bitmap_info_struct(Bitmap0.bmWidth, -Bitmap0.bmHeight, wBitCount).bmiHeader;
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

    fh = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
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


void print_pixel_console(unsigned char r, unsigned char g, unsigned char b) {
    static std::string asciiChars = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";

    auto color = 0;

    int gray = static_cast<int>(0.2989 * r + 0.5870 * g + 0.1140 * b);
    size_t asciiIndex = gray * (asciiChars.size() - 1) / 255;

    std::cout << "\033[38;2;" << static_cast<int>(r) << ";" << static_cast<int>(g) << ";" << static_cast<int>(b) << "m";
    std::cout << asciiChars[asciiIndex];
    std::cout << "\033[0m";
}

bool debug_print_layers(const std::vector<std::vector<Coords>> &layers, HBITMAP &p_hbitmap, const std::vector<COLORREF> &colors) {
    for (auto i = 0; i < layers.size(); ++i){
        debug_print_layer(layers[i], p_hbitmap, colors[i%colors.size()]);
    }
    return true;
}

bool debug_print_layer(const std::vector<Coords> &layer, HBITMAP &p_hbitmap, const COLORREF &color){
    HDC hdc = CreateCompatibleDC(nullptr);
    SelectObject(hdc, p_hbitmap);
    BITMAP bitmap;
    GetObject(p_hbitmap, sizeof(BITMAP), &bitmap);
    int height = bitmap.bmHeight;

    for(auto &coords : layer) {
        SetPixel(hdc, coords.x, height - coords.y, color);
    }
    DeleteDC(hdc);
    return true;
}

void debug_print_grey_background(HBITMAP &canvas, const HBITMAP &p_hbitmap, double d) {
    HDC bg_dc = CreateCompatibleDC(nullptr);
    SelectObject(bg_dc, p_hbitmap);

    HDC canvas_dc = CreateCompatibleDC(nullptr);
    SelectObject(canvas_dc, canvas);

    BITMAP bitmap;
    GetObject(p_hbitmap, sizeof(BITMAP), &bitmap);
    int width = bitmap.bmWidth;
    int height = bitmap.bmHeight;


    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            auto color = GetPixel(bg_dc, i, j);
            auto r = GetRValue(color);
            auto g = GetGValue(color);
            auto b = GetBValue(color);
            auto max = max(r, max(g, b));
            if (r != max) {
                r += (max-r) * d;
            }
            if (g != max) {
                g += (max-g) * d;
            }
            if (b != max) {
                b += (max-b) * d;
            }

            SetPixel(canvas_dc, i, j, RGB(r, g, b));
        }
    }
    DeleteDC(bg_dc);
    DeleteDC(canvas_dc);
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

    BITMAPINFO bmi = create_bitmap_info_struct(region.width, region.height, channels * 8);

    void* bits;
    bitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    if(!bitmap) {
        std::cerr << "Could not create destination bitmap\n";
        DeleteObject(hBmpSource);
        stbi_image_free(img);
        return false;
    }
    HDC hdc = CreateCompatibleDC(nullptr);
    SelectObject(hdc, bitmap);
    for(int y = region.top; y < region.height + region.top; ++y) {
        for(int x = region.left; x < region.left + region.width; ++x) {
            unsigned char* pixel = img + (y * width + x) * channels;
            auto r = pixel[0];
            auto g = pixel[1];
            auto b = pixel[2];
            SetPixel(hdc, x - region.left, y - region.top, RGB(r, g, b));
        }
    }
    stbi_image_free(img);
    DeleteDC(hdc);
    DeleteObject(hBmpSource);
    return true;
}
