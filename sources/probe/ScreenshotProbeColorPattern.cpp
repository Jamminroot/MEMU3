#include <chrono>
#include "../headers/probe/ScreenshotProbeColorPattern.h"
#include <map>

// Extract common initialization code into a helper function
bool **initialize_match_map(int width, int height) {
    bool **match_map = new bool *[width];
    for (int i = 0; i < width; ++i) {
        match_map[i] = new bool[height]{false};
    }
    return match_map;
}

// Another helper function to clean up dynamic arrays
void delete_match_map(bool **match_map, int width) {
    for (int i = 0; i < width; ++i) {
        delete[] match_map[i];
    }
    delete[] match_map;
}

bool ScreenshotProbeColorPattern::probe(const ScreenshotData &screenshot) {
    bool **match_map = initialize_match_map(screenshot.region.width, screenshot.region.height);
    std::vector<Coords> matches;
    std::vector<Coords> handles;
    std::vector<std::vector<Coords>> handle_groups;
    int longest_group_idx = -1;
    auto result = common_probe(screenshot, matches, handles, handle_groups, match_map, longest_group_idx);
    delete_match_map(match_map, screenshot.region.width);
    probe_result = result;
    return probe_result.success;
}

void ScreenshotProbeColorPattern::debug_probe_feature_layers(const ScreenshotData &screenshot,
                                                             std::vector<std::pair<std::string, std::vector<Coords>>> &res) const {
    res = std::vector<std::pair<std::string, std::vector<Coords>>>();
    bool **match_map = initialize_match_map(screenshot.region.width, screenshot.region.height);
    std::vector<Coords> matches;
    std::vector<Coords> handles;
    std::vector<std::vector<Coords>> handle_groups;
    int longest_group_index = -1;
    auto result = common_probe(screenshot, matches, handles, handle_groups, match_map, longest_group_index);
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
        res.emplace_back("Result", std::vector<Coords>());
        Logger::show("Supposed x=" + std::to_string(result.coords.x) + ", y=" + std::to_string(result.coords.y));
        res.back().second.push_back(result.coords);
    } else if (longest_group_index == -1) {
        Logger::show("Longest group pointer is not found!");
    } else if (handle_groups[longest_group_index].empty()) {
        Logger::show("Longest group is empty!");
    }
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

ProbeResult ScreenshotProbeColorPattern::common_probe(const ScreenshotData &screenshot, std::vector<Coords> &matches,
                                                      std::vector<Coords> &handles,
                                                      std::vector<std::vector<Coords>> &handle_groups, bool **match_map,
                                                      int &longest_group_index) const {
    ProbeResult result;
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
    int not_red_threshold = (int) (
            (check_not_red_pixels_area * check_height + check_not_red_pixels_area * check_width) *
            threshold_is_not_red);
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

    handle_groups.emplace_back();
    longest_group_index = -1;

    int longest_group_delta = -1;
    int max_y_for_longest_group = -1;

    auto max_group_distance_y = 3;
    auto max_group_distance_x = 80;
    // Split handles into groups: handles are considered the same group if they are:
    // * Adjacent to each other
    // * Have y coordinate difference less than max_group_distance_y and x coordinate difference less than max_group_distance_x

    std::map<int, std::pair<int, int>> y_to_x_range; // Maps y to a pair of min_x and max_x
    std::map<int, size_t> y_to_group_index; // Maps y to its corresponding group index
    int min_x_for_longest_group = INT_MAX;

    for (const auto &curr_handle: handles) {
        bool group_found = false;

        for (auto fitting_grp_itr = y_to_x_range.lower_bound(curr_handle.y - max_group_distance_y);
             fitting_grp_itr != y_to_x_range.end() &&
             std::abs(fitting_grp_itr->first - curr_handle.y) <= max_group_distance_y;
             ++fitting_grp_itr) {

            auto &[min_x_in_group, max_x_in_group] = fitting_grp_itr->second;
            auto grp_y = fitting_grp_itr->first;

            if (curr_handle.x >= min_x_in_group - max_group_distance_x &&
                curr_handle.x <= max_x_in_group + max_group_distance_x) {
                group_found = true;

                // Update min and max x
                min_x_in_group = min(min_x_in_group, curr_handle.x);
                max_x_in_group = max(max_x_in_group, curr_handle.x);

                y_to_x_range[grp_y] = {min_x_in_group, max_x_in_group};
                size_t curr_group_idx = y_to_group_index[grp_y];
                handle_groups[curr_group_idx].push_back(curr_handle);

                // Update longest group info
                int delta = max_x_in_group - min_x_in_group;
                if (delta > longest_group_delta) {
                    longest_group_delta = delta;
                    longest_group_index = (int) curr_group_idx;

                    max_y_for_longest_group = max(max_y_for_longest_group, curr_handle.y);
                    min_x_for_longest_group = min(min_x_in_group, curr_handle.x);
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

    if (longest_group_index >= 0) {
        result.coords = Coords(min_x_for_longest_group, max_y_for_longest_group);
        result.success = true;
    } else {
        result.success = false;
    }
    return result;
}
