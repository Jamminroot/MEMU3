#include "../headers/Manager.h"
#include "../headers/Utils.h"
#include "../headers/Overlay.h"
#include "../headers/Configuration.h"

#include <condition_variable>
#include <mutex>
#include <fstream>

std::mutex exit_mutex;
std::mutex pause_mutex;
std::condition_variable exit_condition;
std::condition_variable pause_condition;

Manager::~Manager() = default;

bool Manager::is_running() const {
    return running;
}

Manager::Manager() : running(false), exitRequested(false) {

    Configuration config = read_configuration();
    Rect regionSizeAndOffset = Rect(config.scan_width, config.scan_height, config.scan_horizontal_offset, config.scan_vertical_offset);
    screenshot = ScreenshotData(regionSizeAndOffset);
    farHeadOffset = Coords(config.far_offset_x, config.far_offset_y);
    closeHeadOffset = Coords(config.close_offset_x, config.close_offset_y);
    sensitivity = config.sensitivity;
    strength = config.strength;

    RECT desktop;
    const auto hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    screenSize.x = desktop.right;
    screenSize.y = desktop.bottom;
    int left = screenSize.x / 2 + regionSizeAndOffset.left;
    int top = screenSize.y / 2 + regionSizeAndOffset.top;
    region = Rect(regionSizeAndOffset.width, regionSizeAndOffset.height, left, top);
    median.x = region.left - screenSize.x / 2;
    median.y = region.top + region.height - screenSize.y / 2;
    enemyCoords = Coords();
    fill_multiplier_table();
}

bool Manager::is_exit_requested() const {
    return exitRequested;
}

void Manager::update_enemy_coords_with_local_coords(int x, int y) {
    auto dv = (clamp(((float) lastKnownBarSize.x / 2.0f + (float) lastKnownBarSize.y), 5.0f, 15.0f) - 5.0f) / 10.0f;
    enemyCoords.set((int) lerp(dv, (float) closeHeadOffset.x, (float) farHeadOffset.x) + median.x + x,
                    (int) lerp(dv, (float) closeHeadOffset.y, (float) farHeadOffset.y) + median.y - y);
}

bool Manager::is_crosshair_over_enemy() const {
    return enemyCoords.length <= (float) triggerDistanceThreshold;
}

void Manager::update_enemy_coords_with_local_coords(Coords coords) {
    update_enemy_coords_with_local_coords(coords.x, coords.y);
}

void Manager::increase_sensitivity() {
    sensitivity = min(sensitivity + 0.1f, MAXIMUM_SENSITIVITY_VALUE);
    Overlay::toggle_render();
    Overlay::show_hint("Sensitivity: " + to_string(sensitivity));
}

void Manager::decrease_sensitivity() {
    sensitivity = max(sensitivity - 0.1f, 0.1f);
    Overlay::toggle_render();
    Overlay::show_hint("Sensitivity: " + to_string(sensitivity));
}

void Manager::increase_mode_value() {
    switch (mode) {
        case flick:
        case aim:
            strength = min(strength + 0.5f, MAXIMUM_AIM_STRENGTH_VALUE);
            Overlay::show_hint("Strength: " + to_string(strength));
            break;
        case trigger:
            triggerDistanceThreshold = min (++triggerDistanceThreshold, MAXIMUM_TRIGGER_THRESHOLD_VALUE);
            Overlay::show_hint("Distance: " + std::to_string(triggerDistanceThreshold));
            break;
        case hanzo:
            hanzoVerticalOffset = min (++hanzoVerticalOffset, MAXIMUM_HANZO_VERTICAL_OFFSET_VALUE);
            Overlay::show_hint("Offset: " + std::to_string(hanzoVerticalOffset));
            break;
    }

}

void Manager::decrease_mode_value() {
    switch (mode) {
        case flick:
        case aim:
            strength = max(strength - 0.5f, 0.0f);
            Overlay::show_hint("Strength: " + to_string(strength));
            break;
        case trigger:
            triggerDistanceThreshold = max (--triggerDistanceThreshold, 1);
            Overlay::show_hint("Distance: " + std::to_string(triggerDistanceThreshold));
            break;
        case hanzo:
            hanzoVerticalOffset = max (--hanzoVerticalOffset, 0);
            Overlay::show_hint("Offset: " + std::to_string(hanzoVerticalOffset));
            break;
    }
}

void Manager::toggle_mode() {
    mode = (Mode) ((((int) mode) + 1) % sizeof(Mode));
    switch (mode) {
        case hanzo:
            Overlay::show_hint("Mode: Hanzo");
            break;
        case aim:
            Overlay::show_hint("Mode: Aim Assist");
            break;
        case flick:
            Overlay::show_hint("Mode: Flickshots");
            break;
        case trigger:
            Overlay::show_hint("Mode: Triggerbot");
            break;
    }
    Overlay::toggle_render();
}

void Manager::stop_thread_until_exit() const {
    std::unique_lock<std::mutex> lck(exit_mutex);
    while (!is_exit_requested()) exit_condition.wait(lck);
}

void Manager::pause_thread_if_not_running() const {
    std::unique_lock<std::mutex> lck(pause_mutex);
    while (!is_running()) pause_condition.wait(lck);
}

void Manager::request_exit() {
    std::unique_lock<std::mutex> lck(exit_mutex);
    running = false;
    exitRequested = true;
    exit_condition.notify_all();
}

void Manager::set_running(const bool &state, const bool &silent) {
    std::unique_lock<std::mutex> lck(pause_mutex);
    running = state;
    pause_condition.notify_all();
    if (!silent) Overlay::show_hint(running ? "Running" : "Paused");
}

bool Manager::read_next_colorconfig(std::vector<RGBQUAD> &colors, std::string &config) {
    auto configs = list_files_by_mask("*.colorset");
    if (configs.empty()) return false;
    currentColorconfigIndex = (currentColorconfigIndex + 1) % configs.size();
    config = configs[currentColorconfigIndex];
    colors.clear();
    std::string line;
    std::ifstream configFile(config);
    if (configFile.is_open()) {
        while (getline(configFile, line)) {
            colors.push_back(parse_rgbquad_from_string(line));
        }
    }
    return true;
}

std::vector<std::string> Manager::list_files_by_mask(const std::string &mask) {
    std::vector<std::string> configs = std::vector<std::string>();
    try {
        WIN32_FIND_DATAW FindFileData;
        HANDLE hFind;
        hFind = FindFirstFile(s2ws(mask).c_str(), &FindFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string filename = ws2s(FindFileData.cFileName);
                configs.push_back(filename);
            } while (FindNextFile(hFind, &FindFileData));
            FindClose(hFind);
        }
    } catch (...) {
        return configs;
    }
    return configs;
}

RGBQUAD Manager::parse_rgbquad_from_string(const std::string &line) {
    RGBQUAD rgbquad;
    std::vector<std::string> parts = split_string(line, ',');
    rgbquad.rgbBlue = (BYTE) std::stoi(parts.at(0));
    rgbquad.rgbGreen = (BYTE) std::stoi(parts.at(1));
    rgbquad.rgbRed = (BYTE) std::stoi(parts.at(2));
    rgbquad.rgbReserved = (BYTE) std::stoi(parts.at(3));
    return rgbquad;
}

void Manager::toggle_next_colorconfig() {
    auto runningCache = is_running();
    set_running(false, true);
    std::vector<RGBQUAD> colors;
    std::string fname;
    if (read_next_colorconfig(colors, fname)) {
        Overlay::show_hint("Colors config: " + split_string(fname, '.').at(0));
        initialize_color_table(colors, true);
    } else {
        Overlay::show_hint("Can't toggle colorset");
    }
    if (runningCache) {
        set_running(runningCache, true);
    }
}

bool Manager::dump_table(std::string &tablename) const {
    std::ofstream outStream;
    outStream.open(tablename, std::ios::out | std::ios::binary);
    outStream.write((const char *) colorHashTable, sizeof(colorHashTable));
    outStream.close();
    Overlay::show_hint("Saving color scan table to file.");
    return true;
}

bool Manager::restore_table(std::string &tablename) const {
    Overlay::show_hint("Restoring cached color scan table.");
    std::ifstream inStream(tablename);
    size_t chars_read;
    if (!(inStream.read((char *) colorHashTable, sizeof(colorHashTable)))) {
        if (!inStream.eof()) {
            return false;
        }
    }
    chars_read = (size_t)inStream.gcount();
    return chars_read == sizeof(colorHashTable);
}

std::string Manager::hashtable_name(const std::vector<RGBQUAD> &pColors) {
    std::string bytes;
    for (auto targetColor : pColors) {
        bytes.push_back(targetColor.rgbRed);
        bytes.push_back(targetColor.rgbGreen);
        bytes.push_back(targetColor.rgbBlue);
        bytes.push_back(targetColor.rgbReserved);
    }
    return "ct_" + base64_encode(bytes) + ".bin";
}

void Manager::initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile) {
    memset(colorHashTable, '\0', COLOR_HASHTABLE_SIZE);
    auto tablename = hashtable_name(pColors);
    if (pUseCacheFile) {
        if (restore_table(tablename)) {
            return;
        }
    }
    Overlay::show_hint("Building color scan table.");
    int colorIndex = 0;
    for (auto targetColor : pColors) {
        colorIndex++;
        Overlay::show_hint("Color " + std::to_string(colorIndex) + "/" + std::to_string(pColors.size()));
        for (auto i = 0x000000u; i <= 0xFFFFFFu; i++) {
            bool res = probe_bytes_against_rgbquad(((BYTE) ((i & 0xFF0000u) >> 16)), ((BYTE) ((i & 0x00FF00u) >> 8)), (BYTE) (i & 0x0000FFu), targetColor);
            colorHashTable[i / 8] |= (byte) (res << (i % 8));
        }
    }
    dump_table(tablename);
}

bool Manager::probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor) {
    auto dR = r - targetColor.rgbRed;
    auto dG = g - targetColor.rgbGreen;
    auto dB = b - targetColor.rgbBlue;
    auto checkResult = dR * dR + dG * dG + dB * dB <= targetColor.rgbReserved * targetColor.rgbReserved;
    return checkResult;
}

void Manager::fill_multiplier_table() {
    int distance = 0;
    auto bracketSize = 25;
    for (auto i = 0; i < bracketSize; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp(float(distance) / float(bracketSize), 0.1f, 0.2f);
    }
    bracketSize = 50;
    for (auto i = 0; i < bracketSize; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp(float(distance) / float(bracketSize), 0.2f, 1.0f);
    }
    bracketSize = 50;
    for (auto i = 0; i < 50; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp(float(distance) / float(bracketSize), 1.0f, 0.8f);
    }
    for (auto i = distance; i < MULTIPLIER_TABLE_SIZE; ++i) {
        multiplierTable[i] = lerp((float) i / (MULTIPLIER_TABLE_SIZE - 100.0f), 0.8f, 0.1f);
    }
}

Configuration Manager::read_configuration() const {
    Configuration config = Configuration();

    std::string line;
    std::ifstream configFile("MEMU3.config");
    if (configFile.is_open()) {
        while (getline(configFile, line)) {
            parse_config_file_line(config, line);
        }
    }
    return config;
}

bool Manager::parse_config_file_line(Configuration &config, std::string &line) const {
    std::vector<std::string> parts = split_string(line, '=');
    if (parts.size() != 2) {
        return false;
    }
    std::string key = parts[0];
    if (key[0] == '#') {
        return true;
    }
    std::string value = parts[1];

    if (key == "strength") {
        config.strength = (float) atof(value.c_str());
        return true;
    } else if (key == "sensitivity") {
        config.sensitivity = (float) atof(value.c_str());
        return true;
    } else if (key == "far_offset_x") {
        config.far_offset_x = atoi(value.c_str());
        return true;
    } else if (key == "far_offset_y") {
        config.far_offset_y = atoi(value.c_str());
        return true;
    } else if (key == "close_offset_x") {
        config.close_offset_x = atoi(value.c_str());
        return true;
    } else if (key == "close_offset_y") {
        config.close_offset_y = atoi(value.c_str());
        return true;
    } else if (key == "scan_vertical_offset") {
        config.scan_vertical_offset = atoi(value.c_str());
        return true;
    } else if (key == "scan_horizontal_offset") {
        config.scan_horizontal_offset = atoi(value.c_str());
        return true;
    } else if (key == "scan_width") {
        config.scan_width = atoi(value.c_str());
        return true;
    } else if (key == "scan_height") {
        config.scan_height = atoi(value.c_str());
        return true;
    }
    return false;
}
