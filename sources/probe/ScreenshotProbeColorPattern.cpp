#include <iostream>
#include <chrono>
#include "../headers/probe/ScreenshotProbeColorPattern.h"

bool ScreenshotProbeColorPattern::probe(const ScreenshotData &screenshot) {
    bool **match = new bool *[screenshot.region.width];

    for (int i = 0; i < screenshot.region.width; ++i) {
        match[i] = new bool[screenshot.region.height];
    }

    auto matches = std::vector<Coords>();
    for (int y = 0; y < screenshot.region.height; ++y) {
        for (int x = 0; x < screenshot.region.width; ++x) {
            auto color = screenshot.get_pixel(x, y);
            if (probe_color(color)) {
                match[x][y] = true;
                matches.emplace_back(x, y);
            }
        }
    }

    static int bar_width = 15;
    auto height_block = std::vector<bool>(heights.size(), false);
    for(auto coords : matches){
        auto x = coords.x;
        auto y = coords.y;

        for(auto hi = 0; hi < heights.size(); ++hi){
            auto height = heights[hi];
            auto passable_threshold = bar_width * height * (threshold / 100.0);
            auto count = 0;
            for(auto i = 0; i < height; ++i){
                for (auto j = 0; j < bar_width; ++j){
                    if (x + j >= screenshot.region.width || y + i >= screenshot.region.height) {
                        continue;
                    }
                    if (match[x + j][y - i]) {
                        count++;
                    }
                }
            }
            auto is_block =  count >= passable_threshold;
            if (!is_block) {
                break;
            }
            height_block[hi] = true;
        }
    }

    for (int i = 0; i < screenshot.region.width; ++i) {
        delete[] match[i];
    }
    delete[] match;
    return false;
}

std::vector<std::pair<std::string, std::vector<Coords>>> ScreenshotProbeColorPattern::debug_probe_feature_layers(const ScreenshotData &screenshot) const {
    auto res = std::vector<std::pair<std::string, std::vector<Coords>>>();

    bool **match_map = new bool *[screenshot.region.width];

    for (int i = 0; i < screenshot.region.width; ++i) {
        match_map[i] = new bool[screenshot.region.height] {false };
    }
    auto matches = std::vector<Coords>();
    auto handles = std::vector<Coords>();
    // When iterating a screenshot, 0:0 is bottom-left corner
    for (int y = 0; y < screenshot.region.height; ++y) {
        for (int x = 0; x < screenshot.region.width; ++x) {
            auto color = screenshot.get_pixel(x, y);
            if (probe_color(color)) {
                match_map[x][y] = true;
                matches.emplace_back(x, y);
            }
        }
    }

    // Measure time for this block
    auto th = 0.80;
    auto check_not_red_stripes = 3;
    auto check_offset = 3;
    auto check_height = 10;
    auto check_width = 10;
    int is_red_threshold_w = (int)(th * check_width);
    int is_red_threshold_h = (int)(th * check_height);
    int not_red_threshold = (int)((check_not_red_stripes*check_height + check_not_red_stripes*check_width) * 0.50);
    for (auto coords : matches) {
        auto x = coords.x;
        auto y = coords.y;
        if (probe_handle(screenshot, match_map, x, y, is_red_threshold_w, is_red_threshold_h, not_red_threshold, check_height, check_width, check_offset, check_not_red_stripes)) {
            handles.emplace_back(x, y);
        }
    }
    static int bar_width = 10;
    auto height_layers = std::vector<std::vector<Coords>>(heights.size(), std::vector<Coords>());
    auto thresholds = std::vector<double>(heights.size(), 0);
    for(auto hi = 0; hi < heights.size(); ++hi){
        auto height = heights[hi];
        auto passable_threshold = bar_width * height * (threshold / 100.0);
        thresholds[hi] = passable_threshold;
    }
    for(auto coords : handles){
        auto x = coords.x;
        auto y = coords.y;

        for(auto hi = 0; hi < heights.size(); ++hi){
            auto height = heights[hi];
            auto passable_threshold = thresholds[hi];

            auto count = 0;
            for(auto i = 0; i < height; ++i){
                for (auto j = 0; j < bar_width; ++j){
                    auto xx = x + j;
                    auto yy = y - i;
                    if (xx >= screenshot.region.width || yy >= screenshot.region.height || yy < 0) {
                        continue;
                    }
                    if (match_map[xx][yy]) {
                        count++;
                    }
                    if (count >= passable_threshold) {
                        break;
                    }
                }
            }
            auto is_valid_block = count >= passable_threshold;
            if (!is_valid_block) {
                break;
            }
            height_layers[hi].push_back(coords);
        }
    }

    for (int i = 0; i < screenshot.region.width; ++i) {
        delete[] match_map[i];
    }
    delete[] match_map;

    res.emplace_back("0_Reds",matches);
    res.emplace_back("1_Handles", handles);
    for(auto hi = 0; hi< heights.size(); ++hi){
        auto layer = height_layers[hi];
        auto height = heights[hi];
        auto s_height = std::string("0") + std::to_string(height);
        res.emplace_back("2_height_"+ s_height, layer);
    }
    return res;
}

bool ScreenshotProbeColorPattern::probe_handle(const ScreenshotData &screenshot, bool **match_map, int x, int y, int red_threshold_w, int red_threshold_h, int not_red_threshold, int check_height, int check_width, int check_offset, int check_not_red_stripes) {

    // When iterating a screenshot, 0:0 is bottom-left corner
    if (x + check_width >= screenshot.region.width || y + check_height >= screenshot.region.height || x<=check_offset + check_not_red_stripes || y<=check_offset + check_not_red_stripes) {
        return false;
    }

    if (match_map[x-check_offset][y+check_offset]) {
        return false;
    }

    auto count_not_reds = 0;

    // Checking that area to the left and down of the given pixel is not red
    // Given pixel is top-right corner of the area
    for(auto i=1; i<=check_not_red_stripes; ++i){
        for(auto j=1; j<= check_not_red_stripes; ++j){
            if (!match_map[x-i][y-j - check_offset]) {
                count_not_reds++;
            }
        }
    }

    // Checking that area to the right and up of the given pixel is not red
    // Given pixel is bottom-left corner of the area
    for(auto i=0; i<check_width; ++i){
        for(auto j=0; j< check_not_red_stripes; ++j){
            if (!match_map[x+i][y+j+check_offset]) {
                count_not_reds++;
            }
        }
    }

    if (count_not_reds < not_red_threshold) {
        return false;
    }
    auto count = 0;

    // Borders check passed, now checking that the given pixel is red, as well as the area to the right and down of the given pixel (x+1 - to the right, y-1 - to the bottom)

    for(auto i=1; i < check_width; ++i){
        if (match_map[x+i][y]) {
            count++;
        }
    }

    if (count < red_threshold_w) {
        return false;
    }

    count = 0;
    for(auto i=1; i < check_height; ++i){
        if (match_map[x][y-i]) {
            count++;
        }
    }

    if (count < red_threshold_h) {
        return false;
    }

    return true;
}
