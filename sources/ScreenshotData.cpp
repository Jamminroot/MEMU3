#include "../headers/ScreenshotData.h"

ScreenshotData::ScreenshotData(const Rect &regionSize) {
    size = regionSize.width * regionSize.height;
}

ScreenshotData::~ScreenshotData() = default;
