#pragma once
#include "ScreenshotProbeHashTable.h"
class ScreenshotProbeColorPattern : public ScreenshotProbeHashTable {
public:
    explicit ScreenshotProbeColorPattern(std::vector<int> &heights, int threshold = 85) : heights(heights), threshold(threshold) {};
    ~ScreenshotProbeColorPattern() override = default;
    bool probe(const ScreenshotData &screenshot) override;
    static bool probe_handle(const ScreenshotData &screenshot, bool **match_map, int x, int y, int is_red_threshold_w, int is_red_threshold_h, int not_red_threshold, int check_height, int check_width, int check_offset, int check_not_red_stripes);
    void debug_probe_feature_layers(const ScreenshotData &data, std::vector<std::pair<std::string, std::vector<Coords>>> &res) const override;
private:
    ProbeResult common_probe(const ScreenshotData &screenshot, std::vector<Coords>& matches, std::vector<Coords>& handles, std::vector<std::vector<Coords>> &handle_groups, bool **match_map, int &longest_group_idx) const;
    std::vector<int> heights;
    int threshold;
};