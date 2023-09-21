#pragma once
#include "ScreenshotProbeHashTable.h"
class ScreenshotProbeColorPattern : public ScreenshotProbeHashTable {
public:
    ScreenshotProbeColorPattern(std::vector<int> &heights, int threshold = 85) : heights(heights), threshold(threshold) {};
    virtual ~ScreenshotProbeColorPattern() = default;
    virtual bool probe(const ScreenshotData &screenshot) override;
    virtual std::vector<std::vector<Coords>> debug_probe_feature_layers(const ScreenshotData &data) const override;
private:
    std::vector<int> heights;
    int threshold;
};