#include "../headers/ScreenshotData.h"

ScreenshotData::ScreenshotData(const Rect &regionSize) {
    size = regionSize.width * regionSize.height;
}

ScreenshotData::ScreenshotData() {
    size = 0;
}

ScreenshotData::~ScreenshotData() = default;
