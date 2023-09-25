#include "../../headers/probe/ScreenshotProbeHashTableBrute.h"
#include <cmath>

bool ScreenshotProbeHashTableBrute::probe_closest_spiral(const ScreenshotData &screenshot, const int side) {
    auto offset = probe_result.coords.as_offset(screenshot.region.width);
    // Possibly swap region width and height places
    if (fabs(fmin(probe_result.coords.x, screenshot.region.height - probe_result.coords.x) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
    if (fabs(fmin(probe_result.coords.y,  screenshot.region.width - probe_result.coords.y) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
    if (probe_color(screenshot.data[offset])) return true;
    auto directionSign = -1;

    // First 2 steps are hard-coded (outside of for-loops)
    offset += screenshot.region.width;
    if (probe_color(screenshot.data[offset])) return true;
    offset++;
    if (probe_color(screenshot.data[offset])) return true;

    for (auto currentMaxSideSize = 2; currentMaxSideSize <= side; currentMaxSideSize++) {
        for (auto currentSideStep = 1; currentSideStep <= currentMaxSideSize; currentSideStep++) {
            offset += screenshot.region.width * directionSign;
            if (offset < 0 || offset >= screenshot.size) return false;
            if (!probe_color(screenshot.data[offset])) continue;
            return true;
        }

        for (auto currentSideStep = 1; currentSideStep <= currentMaxSideSize; currentSideStep++) {
            offset += directionSign;
            if (offset < 0 || offset >= screenshot.size) return false;
            if (!probe_color(screenshot.data[offset])) continue;
            return true;
        }
        directionSign = -directionSign;
    }
    return false;
}

bool ScreenshotProbeHashTableBrute::probe(const ScreenshotData &screenshot) {

    auto index = probe_result.success ? probe_result.coords.as_offset(screenshot.region.width) : 0;
    auto probe_succeeded = probe_closest_spiral(screenshot, 9);
    if (probe_succeeded && locate_healthbar_handle_left(screenshot, index)) {
        return true;
    }
    auto i = (IGNORED_BORDER_SIZE) + IGNORED_BORDER_SIZE * screenshot.region.width;

    for (auto y = IGNORED_BORDER_SIZE; y < screenshot.region.height - IGNORED_BORDER_SIZE; ++y) {
        for (auto x = IGNORED_BORDER_SIZE; x < screenshot.region.width - IGNORED_BORDER_SIZE; ++x) {
            // Next i - vertically. Unchecked, since we should not exceed that with the loop bounds above.
            i += 1;
            //auto i = coords_to_offset(x, y);
            if (!probe_color(screenshot.data[i])) continue;
            auto xx = x;
            while (xx > IGNORED_BORDER_SIZE && (probe_all_points_diagonal(screenshot, i - 1)) || probe_any_point_left(screenshot, i - 3)) {
                xx--;
                i = screenshot.coords_to_offset(xx, y);
            }
            if (!probe_handle(screenshot, i)) continue;
            screenshot.offset_to_coords(i, probe_result.coords);
            return true;
        }
        // Reset to the beginning of a new line - "0" x (actually + 0+border_size) plus height*width
        i = (IGNORED_BORDER_SIZE) + (y - 1) * screenshot.region.width;
    }
    return false;
}

void ScreenshotProbeHashTableBrute::debug_probe_feature_layers(const ScreenshotData &screenshot, std::vector<std::pair<std::string, std::vector<Coords>>> &result) {
    result = std::vector<std::pair<std::string, std::vector<Coords>>>();
    auto mark_red_layer = std::vector<Coords>();
    auto mark_handle_layer = std::vector<Coords>();
    auto final_result_layer = std::vector<Coords>();
    auto i = (IGNORED_BORDER_SIZE) + IGNORED_BORDER_SIZE * screenshot.region.width;
    for (auto y = IGNORED_BORDER_SIZE; y < screenshot.region.height - IGNORED_BORDER_SIZE; ++y) {
        for (auto x = IGNORED_BORDER_SIZE; x < screenshot.region.width - IGNORED_BORDER_SIZE; ++x) {
            // Next i - vertically. Unchecked, since we should not exceed that with the loop bounds above.
            i += 1;
            //auto i = coords_to_offset(x, y);
            if (!probe_color(screenshot.data[i])) continue;
            Coords coords;
            screenshot.offset_to_coords(i, coords);
            mark_red_layer.push_back(coords);
            auto xx = x;
            while (xx > IGNORED_BORDER_SIZE && (probe_all_points_diagonal(screenshot, i - 1)) || probe_any_point_left(screenshot, i - 3)) {
                xx--;
                i = screenshot.coords_to_offset(xx, y);
            }
            if (!probe_handle(screenshot, i)) continue;
            mark_handle_layer.emplace_back(xx, y);
            if (final_result_layer.empty()) {
                final_result_layer.emplace_back(xx, y);
            }
        }
        // Reset to the beginning of a new line - "0" x (actually + 0+border_size) plus height*width
        i = (IGNORED_BORDER_SIZE) + (y - 1) * screenshot.region.width;
    }
    result.emplace_back("reds",mark_red_layer);
    result.emplace_back("mark_handle", mark_handle_layer);
    result.emplace_back("final_result", final_result_layer);
}
