#include <iostream>
#include <chrono>
#include "../headers/probe/ScreenshotProbeColorPattern.h"
#include <map>

bool ScreenshotProbeColorPattern::probe(const ScreenshotData &screenshot) {
    bool **match_map = new bool *[screenshot.region.width];

    for (int i = 0; i < screenshot.region.width; ++i) {
        match_map[i] = new bool[screenshot.region.height]{false};
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

    std::map<int, std::vector<int>> handles_indices;

    // Measure time for this block
    auto threshold_is_red = 0.80;
    auto threshold_is_not_red = 0.50;
    auto check_not_red_pixels_area = 3;
    auto check_offset = 2;
    auto check_height = 8;
    auto check_width = 8;
    int is_red_threshold_w = (int) (threshold_is_red * check_width) / 2;
    int is_red_threshold_h = (int) (threshold_is_red * check_height);
    int not_red_threshold = (int) ((check_not_red_pixels_area * check_height + check_not_red_pixels_area * check_width) * threshold_is_not_red);
    for (auto coords: matches) {
        auto x = coords.x;
        auto y = coords.y;
        if (probe_handle(screenshot, match_map, x, y, is_red_threshold_w, is_red_threshold_h, not_red_threshold,
                         check_height, check_width, check_offset, check_not_red_pixels_area)) {
            handles.emplace_back(x, y);
            if (handles_indices.find(y) == handles_indices.end()) {
                handles_indices.emplace(y, std::vector<int>());
            }
            handles_indices.at(y).push_back(x);
        }
    }

    std::vector<std::vector<Coords>> handle_groups;

    handle_groups.emplace_back();

    std::pair<int, int> longest_group_range = {-1, -1};
    int longest_group_delta = -1;
    const std::vector<Coords> *longest_group_ptr = nullptr;

    auto max_group_distance_y = 3;
    auto max_group_distance_x = 80;
    // Split handles into groups: handles are considered the same group if they are:
    // * Adjacent to each other
    // * Have y coordinate difference less than max_group_distance_y and x coordinate difference less than max_group_distance_x

    std::map<int, std::pair<int, int>> y_to_x_range; // Maps y to a pair of min_x and max_x

    for (const auto &curr_handle: handles) {
        auto y_it = y_to_x_range.lower_bound(curr_handle.y - max_group_distance_y);

        bool found_in_current_group = false;

        if (y_it != y_to_x_range.end() && std::abs(y_it->first - curr_handle.y) <= max_group_distance_y) {
            auto &[min_x, max_x] = y_it->second;

            if (curr_handle.x >= min_x - max_group_distance_x && curr_handle.x <= max_x + max_group_distance_x) {
                found_in_current_group = true;

                // Update min and max x for this y
                min_x = min(min_x, curr_handle.x);
                max_x = max(max_x, curr_handle.x);
            }
        }

        if (found_in_current_group) {
            handle_groups.back().push_back(curr_handle);

            auto &[min_x, max_x] = y_to_x_range[curr_handle.y];
            int current_group_delta = max_x - min_x;

            if (current_group_delta > longest_group_delta) {
                longest_group_delta = current_group_delta;
                longest_group_range = {min_x, max_x};
                longest_group_ptr = &handle_groups.back();
            }
        } else {
            handle_groups.emplace_back(std::vector<Coords>());
            handle_groups.back().push_back(curr_handle);
            y_to_x_range[curr_handle.y] = {curr_handle.x, curr_handle.x};
        }
    }

    static int bar_width = 10;
    auto height_layers = std::vector<std::vector<Coords>>(heights.size(), std::vector<Coords>());
    auto thresholds = std::vector<double>(heights.size(), 0);
    for (auto hi = 0; hi < heights.size(); ++hi) {
        auto height = heights[hi];
        auto passable_threshold = bar_width * height * (threshold / 100.0);
        thresholds[hi] = passable_threshold;
    }


    for (int i = 0; i < screenshot.region.width; ++i) {
        delete[] match_map[i];
    }
    delete[] match_map;

    return false;
}

std::vector<std::pair<std::string, std::vector<Coords>>>
ScreenshotProbeColorPattern::debug_probe_feature_layers(const ScreenshotData &screenshot) const {
    auto res = std::vector<std::pair<std::string, std::vector<Coords>>>();

    bool **match_map = new bool *[screenshot.region.width];

    for (int i = 0; i < screenshot.region.width; ++i) {
        match_map[i] = new bool[screenshot.region.height]{false};
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

    std::map<int, std::vector<int>> handles_indices;

    // Measure time for this block
    auto threshold_is_red = 0.80;
    auto threshold_is_not_red = 0.50;
    auto check_not_red_pixels_area = 3;
    auto check_offset = 2;
    auto check_height = 8;
    auto check_width = 8;
    int is_red_threshold_w = (int) (threshold_is_red * check_width) / 2;
    int is_red_threshold_h = (int) (threshold_is_red * check_height);
    int not_red_threshold = (int) ((check_not_red_pixels_area * check_height + check_not_red_pixels_area * check_width) * threshold_is_not_red);
    for (auto coords: matches) {
        auto x = coords.x;
        auto y = coords.y;
        if (probe_handle(screenshot, match_map, x, y, is_red_threshold_w, is_red_threshold_h, not_red_threshold,
                         check_height, check_width, check_offset, check_not_red_pixels_area)) {
            handles.emplace_back(x, y);
            if (handles_indices.find(y) == handles_indices.end()) {
                handles_indices.emplace(y, std::vector<int>());
            }
            handles_indices.at(y).push_back(x);
        }
    }

    std::vector<std::vector<Coords>> handle_groups;

    handle_groups.emplace_back();

    int longest_group_index = 0;

    int longest_group_delta = -1;
    int max_y_for_longest_group = -1;

    auto max_group_distance_y = 3;
    auto max_group_distance_x = 80;
    // Split handles into groups: handles are considered the same group if they are:
    // * Adjacent to each other
    // * Have y coordinate difference less than max_group_distance_y and x coordinate difference less than max_group_distance_x

    std::map<int, std::pair<int, int>> y_to_x_range; // Maps y to a pair of min_x and max_x
    std::map<int, int> y_to_group_index; // Maps y to its corresponding group index
    int min_x_for_longest_group = INT_MAX;

    for (const auto &curr_handle: handles) {
        bool group_found = false;

        for (auto fitting_grp_itr = y_to_x_range.lower_bound(curr_handle.y - max_group_distance_y);
             fitting_grp_itr != y_to_x_range.end() && std::abs(fitting_grp_itr->first - curr_handle.y) <= max_group_distance_y;
             ++fitting_grp_itr) {

            auto &[min_x_in_group, max_x_in_group] = fitting_grp_itr->second;
            auto grp_y = fitting_grp_itr->first;

            if (curr_handle.x >= min_x_in_group - max_group_distance_x && curr_handle.x <= max_x_in_group + max_group_distance_x) {
                group_found = true;

                // Update min and max x
                min_x_in_group = min(min_x_in_group, curr_handle.x);
                max_x_in_group = max(max_x_in_group, curr_handle.x);

                y_to_x_range[grp_y] = {min_x_in_group, max_x_in_group};
                int curr_group_idx = y_to_group_index[grp_y];
                handle_groups[curr_group_idx].push_back(curr_handle);

                // Update longest group info
                int delta = max_x_in_group - min_x_in_group;
                if (delta > longest_group_delta) {
                    longest_group_delta = delta;
                    longest_group_index = curr_group_idx;

                    max_y_for_longest_group = max(max_y_for_longest_group, curr_handle.y);
                    min_x_for_longest_group = min_x_in_group;  // Update min_x for longest group ONLY here
                }

                break;
            }
        }

        if (!group_found) {
            handle_groups.emplace_back(std::vector<Coords>{curr_handle});
            y_to_x_range[curr_handle.y] = {curr_handle.x, curr_handle.x};
            y_to_group_index[curr_handle.y] = handle_groups.size() - 1;
        }
    }

    static int bar_width = 10;
    auto height_layers = std::vector<std::vector<Coords>>(heights.size(), std::vector<Coords>());
    auto thresholds = std::vector<double>(heights.size(), 0);
    for (auto hi = 0; hi < heights.size(); ++hi) {
        auto height = heights[hi];
        auto passable_threshold = bar_width * height * (threshold / 100.0);
        thresholds[hi] = passable_threshold;
    }

    for (int i = 0; i < screenshot.region.width; ++i) {
        delete[] match_map[i];
    }
    delete[] match_map;

    res.emplace_back("0_Reds", matches);
    res.emplace_back("1_Handles", handles);

    auto idx = 0;
    for (auto grp: handle_groups) {
        auto s_idx = std::string("0") + std::to_string(idx);
        res.emplace_back("2_group(" + s_idx + ")", grp);
        idx++;
    }
    if (longest_group_index >= 0 && !handle_groups[longest_group_index].empty()) {
        res.emplace_back("3_longest_grp", handle_groups[longest_group_index]);
    }  else if (longest_group_index == -1) {
        std::cerr<<"Longest group pointer is not found!"<<std::endl;
    } else if (handle_groups[longest_group_index].empty()) {
        std::cerr<<"Longest group is empty!"<<std::endl;
    }
    res.emplace_back("Result", std::vector<Coords>());
    std::cout << "Supposed x=" << min_x_for_longest_group << ", y=" << max_y_for_longest_group << std::endl;
    res.back().second.emplace_back(min_x_for_longest_group, max_y_for_longest_group);
    return res;
}

bool ScreenshotProbeColorPattern::probe_handle(const ScreenshotData &screenshot, bool **match_map, int x, int y,
                                               int red_threshold_w, int red_threshold_h, int not_red_threshold,
                                               int check_height, int check_width, int check_offset,
                                               int check_not_red_stripes) {

    // When iterating a screenshot, 0:0 is bottom-left corner
    if (x + check_width >= screenshot.region.width || y + check_height >= screenshot.region.height ||
        x <= check_offset + check_not_red_stripes || y <= check_offset + check_not_red_stripes) {
        return false;
    }

/*
    if (match_map[x-check_offset][y+check_offset]) {
        return false;
    }
*/

    auto count_not_reds = 0;

    // Checking that area to the left and down of the given pixel is not red
    // Given pixel is top-right corner of the area
    for (auto i = 1; i <= check_not_red_stripes; ++i) {
        for (auto j = 1; j <= check_not_red_stripes; ++j) {
            if (!match_map[x - i][y - j - check_offset]) {
                count_not_reds++;
            }
        }
    }

    // Checking that area to the right and up of the given pixel is not red
    // Given pixel is bottom-left corner of the area
    for (auto i = 0; i < check_width; ++i) {
        for (auto j = 0; j < check_not_red_stripes; ++j) {
            if (!match_map[x + i][y + j + check_offset]) {
                count_not_reds++;
            }
        }
    }

    if (count_not_reds < not_red_threshold) {
        return false;
    }
    auto count = 0;

    // Borders check passed, now checking that the given pixel is red, as well as the area to the right and down of the given pixel (x+1 - to the right, y-1 - to the bottom)

    for (auto i = 1; i < check_width; ++i) {
        if (match_map[x + i][y]) {
            count++;
        }
    }

    if (count < red_threshold_w) {
        return false;
    }

    count = 0;
    for (auto i = 1; i < check_height; ++i) {
        if (match_map[x][y - i]) {
            count++;
        }
    }

    if (count < red_threshold_h) {
        return false;
    }

    return true;
}
