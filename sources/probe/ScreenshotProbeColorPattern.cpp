#include "../headers/probe/ScreenshotProbeColorPattern.h"
bool ScreenshotProbeColorPattern::probe(const ScreenshotData &screenshot) {
    bool* match = new bool[screenshot.size];

    for (int y = 0; y < screenshot.region.height; ++y) {
        for (int x = 0; x < screenshot.region.width; ++x) {
            auto color = screenshot.get_pixel(x, y);
            if (probe_color(color)) {
                match[screenshot.coords_to_offset(x, y)] = true;
            }
        }
    }

    delete[] match;
    return false;
}

std::vector<std::vector<Coords>> ScreenshotProbeColorPattern::debug_probe_feature_layers(const ScreenshotData &screenshot) const {
    auto res = std::vector<std::vector<Coords>>();
    auto match_layer = std::vector<Coords>();

    for (int y = 0; y < screenshot.region.height; ++y) {
        for (int x = 0; x < screenshot.region.width; ++x) {
            auto color = screenshot.get_pixel(x, y);
            if (probe_color(color)) {
                match_layer.emplace_back(x, y);
            }
        }
    }
    res.emplace_back(match_layer);
    return res;
}
