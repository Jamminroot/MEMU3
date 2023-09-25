#include <chrono>
#include "../headers/probe/ScreenshotProbeColorPattern.h"
#include <map>
#include <unordered_set>

bool ScreenshotProbeColorPattern::probe(const ScreenshotData &screenshot) {
    int longest_group_idx = -1;
    auto result = common_probe(screenshot, longest_group_idx);
    probe_result = result;
    return probe_result.success;
}

struct GroupData {
    int unique_xs;
    int min_x;
    int max_x;
    int max_y;

    int get_delta() {
        return max_x - min_x;
    }
};

void ScreenshotProbeColorPattern::debug_probe_feature_layers(const ScreenshotData &screenshot,
                                                             std::vector<std::pair<std::string, std::vector<Coords>>> &res) {
    res = std::vector<std::pair<std::string, std::vector<Coords>>>();

    int longest_group_index = -1;
    auto result = common_probe(screenshot, longest_group_index);
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

bool ScreenshotProbeColorPattern::probe_handle(const ScreenshotData &screenshot, int x, int y,
                                               int red_threshold_w, int red_threshold_h, int not_red_threshold,
                                               int check_height, int check_width, int check_offset,
                                               int check_not_red_stripes) {

    // When iterating a screenshot, 0:0 is bottom-left corner
    if (x + check_width + check_not_red_stripes >= screenshot.region.width || y + check_height + check_not_red_stripes >= screenshot.region.height ||
        x < check_width + check_not_red_stripes || y < check_height + check_not_red_stripes) {
        return false;
    }


    if (match_map[x-check_offset][y+check_offset]) {
        return false;
    }


    auto count_not_reds = 0;

    for (auto ox = x-check_not_red_stripes; ox<=x+check_width-check_not_red_stripes; ++ox){
        for (auto oy = y-check_height+check_not_red_stripes; oy <= y + check_not_red_stripes; ++oy) {
            if (ox>=x && oy <= y) {
                continue;
            }
            if (!match_map[ox][oy]) {
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

ProbeResult ScreenshotProbeColorPattern::common_probe(const ScreenshotData &screenshot, int &longest_group_index) {
    ProbeResult result;
    if (match_map == nullptr) {
        init(screenshot.region);
    }
    matches.clear();
    handles.clear();
    handle_groups.clear();

    // When iterating a screenshot, 0:0 is bottom-left corner
    for (int y = 0; y < screenshot.region.height; ++y) {
        for (int x = 0; x < screenshot.region.width; ++x) {
            auto color = screenshot.get_pixel(x, y);
            if (probe_color(color)) {
                match_map[x][y] = true;
                matches.emplace_back(x, y);
            } else {
                match_map[x][y] = false;
            }
        }
    }


    // Measure time for this block
    auto threshold_is_red = 0.85;
    auto threshold_is_not_red = 0.50;
    auto check_not_red_pixels_area = 5;
    auto check_offset = 2;
    auto check_height = 8;
    auto check_width = 8;
    int is_red_threshold_w = (int) (threshold_is_red * check_width) / 2;
    int is_red_threshold_h = (int) (threshold_is_red * check_height);
    int not_red_threshold = (int) ((check_not_red_pixels_area * check_height + check_not_red_pixels_area * check_width) *threshold_is_not_red);
    for (auto coords: matches) {
        auto x = coords.x;
        auto y = coords.y;
        if (probe_handle(screenshot, x, y, is_red_threshold_w, is_red_threshold_h, not_red_threshold,
                         check_height, check_width, check_offset, check_not_red_pixels_area)) {
            handles.emplace_back(coords);
        }
    }

    handle_groups.emplace_back();
    longest_group_index = -1;


    auto max_group_distance_y = 3;
    auto max_group_distance_x = 80;
    // Split handles into groups: handles are considered the same group if they are:
    // * Adjacent to each other
    // * Have y coordinate difference less than max_group_distance_y and x coordinate difference less than max_group_distance_x

    std::map<int, std::pair<int, int>> y_to_x_range; // Maps y to min_x-max_x pair
    if (!handles.empty()) {
        y_to_x_range.emplace(handles[0].y, std::make_pair(handles[0].x, handles[0].x));
    }
    std::map<int, size_t> y_to_group_index; // Maps y to its corresponding group index

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

    std::vector<GroupData> group_datas = std::vector<GroupData>();
    for (auto &group: handle_groups) {
        auto group_length = 0;
        std::unordered_set<int> group_heights;
        auto min_x = INT_MAX;
        auto max_x = INT_MIN;
        auto max_y = INT_MIN;
        // Increase length if group has a handle with x which has not yet appeared in the group
        for (auto &handle: group) {
            if (group_heights.find(handle.x) == group_heights.end()) {
                group_length++;
                group_heights.insert(handle.x);
            }
            min_x = min(min_x, handle.x);
            max_x = max(max_x, handle.x);
            max_y = max(max_y, handle.y);
        }
        group_datas.emplace_back(GroupData{(int) group_heights.size(), min_x, max_x, max_y});
    }


    auto top_result = GroupData{0, 0, 0, 0};

    // Get top result from group_datas
    auto idx = -1;
    for (auto &group_data: group_datas) {
        idx++;
        if (group_data.unique_xs > top_result.unique_xs) {
            longest_group_index = idx;
            top_result = group_data;
        } else if (group_data.unique_xs == top_result.unique_xs) {
            if (group_data.get_delta() < top_result.get_delta()) {
                longest_group_index = idx;
                top_result = group_data;
            } else if (group_data.get_delta() == top_result.get_delta()) {
                if (group_data.max_y < top_result.max_y) {
                    longest_group_index = idx;
                    top_result = group_data;
                }
            }
        }
    }


    if (longest_group_index >= 0) {
        result.coords = Coords(top_result.min_x, top_result.max_y);
        result.success = top_result.unique_xs > 1 && top_result.get_delta() >= 5;
    } else {
        result.success = false;
    }
    return result;
}

ScreenshotProbeColorPattern::ScreenshotProbeColorPattern(std::vector<int> &heights, int threshold) : heights(heights),
                                                                                                     threshold(
                                                                                                             threshold) {
}

void ScreenshotProbeColorPattern::delete_match_map(bool **match_map, const Rect &size) {
    if (match_map == nullptr) {
        return;
    }
    for (int i = 0; i < size.width; ++i) {
        delete[] match_map[i];
    }
    delete[] match_map;
}

void ScreenshotProbeColorPattern::init(const Rect &p_size) {
    if (match_map != nullptr) {
        delete_match_map(match_map, size);
    }
    size = p_size;
    match_map = new bool *[size.width];
    for (int i = 0; i < size.width; ++i) {
        match_map[i] = new bool[size.height]{false};
    }
}

ScreenshotProbeColorPattern::~ScreenshotProbeColorPattern() {
    delete_match_map(match_map, size);
}
