#pragma once
#include "ScreenshotProbe.h"
class ScreenshotProbeColorPattern : public ScreenshotProbe {
public:
    ScreenshotProbeColorPattern() = default;
    virtual ~ScreenshotProbeColorPattern() = default;
    virtual bool probe(const ScreenshotData &data, Coords &enemyCoords) const = 0;
};