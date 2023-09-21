#include "../headers/ScreenshotData.h"

ScreenshotData::ScreenshotData(const Rect &regionSize) {
    size = regionSize.width * regionSize.height;
    region = regionSize;
}

ScreenshotData::ScreenshotData() {
    size = 0;
    region = Rect();
}

RGBQUAD ScreenshotData::get_pixel(int x, int y) const {
    return data[coords_to_offset(x, y)];
}

void ScreenshotData::offset_to_coords(const int &offset, Coords &coords) const {
    coords.x = offset % region.width;
    coords.y = offset / region.width;
}

int ScreenshotData::coords_to_offset(const int &pX, const int &pY) const {
    return pX + pY * region.width;
}

void ScreenshotData::offset_to_coords_inverted_y(const int &offset, Coords &coords) const {
    coords.x = offset % region.width;
    coords.y = region.height - offset / region.width;
}

ScreenshotData::~ScreenshotData() = default;
