#pragma once
#include "ScreenshotProbeHashTable.h"
#include "../Rect.h"

class ScreenshotProbeHashTableBrute : public ScreenshotProbeHashTable {
public:
    ScreenshotProbeHashTableBrute() = default;
    bool probe(const ScreenshotData &screenshot) override;
    bool probe_closest_spiral(const ScreenshotData &screenshot, const int side);
    void debug_probe_feature_layers(const ScreenshotData &screenshot, std::vector<std::pair<std::string, std::vector<Coords>>> &res) const override;
};
