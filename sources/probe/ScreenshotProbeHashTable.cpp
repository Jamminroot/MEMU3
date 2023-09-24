#include "../headers/probe/ScreenshotProbeHashTable.h"

bool ScreenshotProbeHashTable::restore_table(std::string &tablename) const {
    Logger::show("Restoring cached color scan table.");
    std::ifstream inStream(tablename);
    size_t chars_read;
    if (!(inStream.read((char *) hashTable, sizeof(hashTable)))) {
        if (!inStream.eof()) {
            return false;
        }
    }
    chars_read = (size_t) inStream.gcount();
    return chars_read == sizeof(hashTable);
}

bool ScreenshotProbeHashTable::dump_table(std::string &tablename) const {
    std::ofstream outStream;
    outStream.open(tablename, std::ios::out | std::ios::binary);
    outStream.write((const char *) hashTable, sizeof(hashTable));
    outStream.close();
    Logger::show("Saving color scan table to file.");
    return true;
}

ScreenshotProbeHashTable::ScreenshotProbeHashTable() {

    std::vector<RGBQUAD> colors = {
            {65,  38,  240, 19},
            {114, 81,  235, 19},
            {105, 70,  227, 19},
            {145, 124, 253, 19},
            {133, 99,  239, 19},
            {111, 99,  223, 19},
            {115, 103, 229, 19},
            {58,  54,  219, 19},
            {60,  58,  224, 19},
            {46,  23,  212, 19},
            {48,  41,  211, 19},
            {0,   10,  221, 19},
            {80,  60,  248, 27},
    };
    initialize_color_table(colors, true);
}

void ScreenshotProbeHashTable::initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile) {
    memset(hashTable, '\0', COLOR_HASHTABLE_SIZE);
    auto tablename = hashtable_name(pColors);
    if (pUseCacheFile) {
        if (restore_table(tablename)) {
            auto restored_count = 0;
            for(auto i = 0x000000u; i <= 0xFFFFFFu; i++){
                restored_count += (hashTable[i / 8] >> (i % 8)) & 1;
            }
            Logger::show("Restored " + std::to_string(restored_count) + " color variations from cache.");
            return;
        }
    }
    Logger::show("Building color scan table.");
    int colorIndex = 0;
    for (auto targetColor: pColors) {
        colorIndex++;
        Logger::show("Color " + std::to_string(colorIndex) + "/" + std::to_string(pColors.size()));
        for (auto i = 0x000000u; i <= 0xFFFFFFu; i++) {
            bool res = probe_bytes_against_rgbquad(((BYTE) ((i & 0xFF0000u) >> 16)), ((BYTE) ((i & 0x00FF00u) >> 8)),
                                                   (BYTE) (i & 0x0000FFu), targetColor);
            hashTable[i / 8] |= (byte) (res << (i % 8));
        }
    }
    dump_table(tablename);
}

bool ScreenshotProbeHashTable::probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b,
                                                           const RGBQUAD targetColor) {
    auto dR = r - targetColor.rgbRed;
    auto dG = g - targetColor.rgbGreen;
    auto dB = b - targetColor.rgbBlue;
    auto checkResult = dR * dR + dG * dG + dB * dB <= targetColor.rgbReserved * targetColor.rgbReserved;
    return checkResult;
}

bool ScreenshotProbeHashTable::probe_color(const RGBQUAD &color) const {
    auto v = (color.rgbRed << 16) | (color.rgbGreen << 8) | color.rgbBlue;
    return (hashTable[v / 8] & ~(1 << (v % 8))) != 0;
}

bool ScreenshotProbeHashTable::probe_all_points_diagonal(const ScreenshotData &screenshot, const int &offset, const int pLineSize) const {
    auto check = 0;
    for (auto ni = 0; ni < pLineSize; ni++) {
        auto i = offset + (screenshot.region.width * ni - ni);
        auto res = probe_color(screenshot.data[i]);
        check += res;
    }
    return check / CHECK_COEFFICIENT >= SCANNING_THRESHOLD_PERCENT;
}

bool ScreenshotProbeHashTable::probe_any_point_left(const ScreenshotData &screenshot, const int &offset, const int pLineSize) const {
    for (auto ni = 0; ni < pLineSize; ni++) {
        auto i = offset + (screenshot.region.width * ni - ni);
        if (probe_color(screenshot.data[i]))
            return true;
    }
    return false;
}

bool ScreenshotProbeHashTable::probe_handle(const ScreenshotData &screenshot, const int &index) const {
    auto successfulChecks = 0;
    for (auto nx = 0; nx < IGNORED_BORDER_SIZE; nx++)
        for (auto ny = 0; ny < IGNORED_BORDER_SIZE; ny++) {
            // Y axis is inverted (starts at bottom-left); Also healthbar is slash-oriented (`/`), thus X is adjusted by neighbor-Y arrayOffset
            auto i = index - ny * screenshot.region.width + nx - ny;
            successfulChecks += probe_color(screenshot.data[i]);
        }

    return successfulChecks * CHECK_COEFFICIENT >= SCANNING_THRESHOLD_PERCENT;
}

bool ScreenshotProbeHashTable::locate_healthbar_handle_left(const ScreenshotData& screenshot, int &offset) const {
    auto inputX = offset % screenshot.region.width;
    auto inputY = offset / screenshot.region.width;
    auto found = false;
    for (auto xi = inputX; xi > IGNORED_BORDER_SIZE + 2; xi--) {
        auto newOffset = screenshot.coords_to_offset(xi, inputY);
        if (!probe_handle(screenshot, newOffset)) continue;
        offset = newOffset;
        found = true;
    }
    return found;
}

void ScreenshotProbeHashTable::find_healthbar_height(const ScreenshotData& screenshot) {
    auto offset =  probe_result.coords.as_offset(screenshot.region.width);
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 20; ni++) {
        auto i = offset + ni;
        try {
            auto res = probe_color(screenshot.data[i]) || probe_color(screenshot.data[i + screenshot.region.width - 1]) ||
                       probe_color(screenshot.data[i + screenshot.region.width * 2 - 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        } catch (...) {
        }
    }
    lastKnownBarSize.x = check;
}

void ScreenshotProbeHashTable::find_healthbar_width(const ScreenshotData& screenshot) {
    auto offset = probe_result.coords.as_offset(screenshot.region.width);
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 15; ni++) {
        auto i = offset + screenshot.region.width * ni;
        try {
            auto res = probe_color(screenshot.data[i]) || probe_color(screenshot.data[i + 1]) || probe_color(screenshot.data[i + 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        } catch (...) {
        }
    }
    lastKnownBarSize.y = check;
}