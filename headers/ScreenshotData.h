#pragma once

#include <Windows.h>
#include "Rect.h"
#include "Coords.h"

class ScreenshotData {
public:
    ScreenshotData();
    ScreenshotData(const Rect &regionSize);
    ~ScreenshotData();
    RGBQUAD data[600 * 500]{0};
    Rect region;
    int size;

    int coords_to_offset(const int &pX, const int &pY) const {
        return pX + pY * region.width;
    }

    void offset_to_coords(const int &offset, Coords& coords) const {
        coords.x = offset % region.width;
        coords.y = offset / region.width;
    }
};