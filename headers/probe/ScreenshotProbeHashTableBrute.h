#pragma once
#include "ScreenshotProbeHashTable.h"
#include "../Rect.h"

class ScreenshotProbeHashTableBrute : public ScreenshotProbeHashTable {
public:
    ScreenshotProbeHashTableBrute() = default;
    bool probe(const ScreenshotData &screenshot) override;
    bool probe_closest_spiral(const ScreenshotData &screenshot, const int side);
    std::vector<std::pair<std::string, std::vector<Coords>>> debug_probe_feature_layers(const ScreenshotData &screenshot) const override;
};
