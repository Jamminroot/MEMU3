#include "../headers/ScreenshotData.h"

ScreenshotData::ScreenshotData(const Rect &regionSize) {
    size = regionSize.width * regionSize.height;
    region = regionSize;
}

ScreenshotData::ScreenshotData() {
    size = 0;
    region = Rect();
}

ScreenshotData::~ScreenshotData() = default;
