#pragma once
#include "ScreenshotProbeHashTable.h"
class ScreenshotProbeColorPattern : public ScreenshotProbeHashTable {
public:
    explicit ScreenshotProbeColorPattern(std::vector<int> &heights, int threshold = 85);
    ~ScreenshotProbeColorPattern() override;;
    bool probe(const ScreenshotData &screenshot) override;
    bool probe_handle(const ScreenshotData &screenshot, int x, int y, int is_red_threshold_w, int is_red_threshold_h, int not_red_threshold, int check_height, int check_width, int check_offset, int check_not_red_stripes);

    void debug_probe_feature_layers(const ScreenshotData &data, std::vector<std::pair<std::string, std::vector<Coords>>> &res) override;
private:
    ProbeResult common_probe(const ScreenshotData &screenshot, int &longest_group_idx);
    void init(const Rect &size);
    static void delete_match_map(bool **match_map, const Rect &size);
    Rect size;
    std::vector<Coords> matches;
    std::vector<Coords> handles;
    std::vector<std::vector<Coords>> handle_groups;
    std::vector<int> heights;
    int threshold;
    bool **match_map = nullptr;
};