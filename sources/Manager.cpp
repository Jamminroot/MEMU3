#include "../headers/Manager.h"
#include "../headers/Utils.h"
#include "../headers/Overlay.h"
#include "../headers/logging/logger.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"

#include <mutex>
#include <fstream>
#include <cmath>

std::mutex exit_mutex;
std::mutex pause_mutex;
std::condition_variable exit_condition;
std::condition_variable pause_condition;

Manager::~Manager() = default;

bool Manager::is_running() const {
    return running;
}

Manager::Manager(std::unique_ptr<ScreenshotProbe> probe) : running(false), exitRequested(false),
                                                           screenshotProbe(std::move(probe)) {
    read_configuration(config);
    Rect regionSizeAndOffset = Rect(config.scan_width, config.scan_height, config.scan_horizontal_offset,
                                    config.scan_vertical_offset);
    screenshot = ScreenshotData(regionSizeAndOffset);
    farHeadOffset = Coords(config.far_offset_x, config.far_offset_y);
    closeHeadOffset = Coords(config.close_offset_x, config.close_offset_y);

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
    enemyCoords.set((int) lerp_value(dv, (float) closeHeadOffset.x, (float) farHeadOffset.x) + median.x + x,
                    (int) lerp_value(dv, (float) closeHeadOffset.y, (float) farHeadOffset.y) + median.y - y);
}

bool Manager::is_crosshair_over_enemy() const {
    return enemyCoords.vector_length <= (float) triggerDistanceThreshold;
}

void Manager::update_enemy_coords_with_local_coords(Coords coords) {
    update_enemy_coords_with_local_coords(coords.x, coords.y);
}

void Manager::increase_sensitivity() {
    auto new_val = fmin(config.sensitivity + 0.1f, MAXIMUM_SENSITIVITY_VALUE);
    Overlay::toggle_render();
    if (new_val == config.sensitivity) return;
    config.sensitivity = new_val;
    save_config();
    Logger::show("Sensitivity: " + to_string(config.sensitivity));
}

void Manager::decrease_sensitivity() {
    auto new_val = fmax(config.sensitivity - 0.1f, 0.1f);
    Overlay::toggle_render();
    if (new_val == config.sensitivity) return;
    config.sensitivity = new_val;
    save_config();
    Logger::show("Sensitivity: " + to_string(config.sensitivity));
}

void Manager::increase_mode_value() {
    switch (mode) {
        case flick:
        case aim: {
            auto new_val = fmin(config.strength + 0.1f, MAXIMUM_AIM_STRENGTH_VALUE);
            if (new_val == config.strength)
                break;
            config.strength = new_val;
            save_config();
            Logger::show("Strength: " + to_string(config.strength));
            break;
        }
        case trigger: {
            auto new_val = (int) fmin(++triggerDistanceThreshold, MAXIMUM_TRIGGER_THRESHOLD_VALUE);
            if (new_val == triggerDistanceThreshold)
                break;
            triggerDistanceThreshold = new_val;
            save_config();
            Logger::show("Distance: " + std::to_string(triggerDistanceThreshold));
            break;
        }
        case hanzo: {
            auto new_val = (int) fmin(++hanzoVerticalOffset, MAXIMUM_HANZO_VERTICAL_OFFSET_VALUE);
            if (new_val == hanzoVerticalOffset)
                break;
            hanzoVerticalOffset = new_val;
            save_config();
            Logger::show("Offset: " + std::to_string(hanzoVerticalOffset));
            break;
        }
    }
}

void Manager::decrease_mode_value() {
    switch (mode) {
        case flick:
        case aim: {
            auto new_val = fmax(config.strength - 0.1f, 0.1f);
            if (new_val == config.strength)
                break;
            config.strength = new_val;
            save_config();
            Logger::show("Strength: " + to_string(config.strength));
            break;
        }
        case trigger: {
            auto new_val = (int) fmax(--triggerDistanceThreshold, 1);
            if (new_val == triggerDistanceThreshold)
                break;
            triggerDistanceThreshold = new_val;
            save_config();
            Logger::show("Distance: " + std::to_string(triggerDistanceThreshold));
            break;
        }
        case hanzo: {
            auto new_val = (int) fmax(--hanzoVerticalOffset, 0);
            if (new_val == hanzoVerticalOffset)
                break;
            hanzoVerticalOffset = new_val;
            save_config();
            Logger::show("Offset: " + std::to_string(hanzoVerticalOffset));
            break;
        }
    }
}

void Manager::toggle_mode() {
    mode = (Mode) ((((int) mode) + 1) % sizeof(Mode));
    switch (mode) {
        case hanzo:
            Logger::show("Mode: Hanzo");
            break;
        case aim:
            Logger::show("Mode: Aim Assist");
            break;
        case flick:
            Logger::show("Mode: Flickshots");
            break;
        case trigger:
            Logger::show("Mode: Triggerbot");
            break;
    }
    Overlay::toggle_render();
}

void Manager::stop_thread_until_exit(HANDLE &handle) const {
    std::unique_lock<std::mutex> lck(exit_mutex);
    while (!is_exit_requested()) exit_condition.wait(lck);
    ReleaseMutex(handle);
    CloseHandle(handle);
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
    if (!silent) Logger::show(running ? "Running" : "Paused");
}

bool Manager::read_next_colorconfig(std::vector<RGBQUAD> &colors, std::string &config) {
    auto configs = list_files_by_mask(".colorset");
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

bool Manager::read_next_strength_map(std::string &message) {
    auto maps = list_files_by_mask(".bmp", "maps");
    if (maps.empty()) {
        message = "No strength maps found";
        return false;
    }

    currentStrengthMapIndex = (currentStrengthMapIndex + 1) % maps.size();
    auto map = maps[currentStrengthMapIndex];
    std::string line;
    std::ifstream bmpFile("maps/" + map);
    int distribution[255]{0};

    HBITMAP hBitmap = (HBITMAP) LoadImageA(NULL, ("maps/" + map).c_str(), IMAGE_BITMAP, 0, 0,
                                           LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBitmap == NULL) {
#if DEBUG
        Logger::show("Failed to open strength map: "+map);
#endif
        message = "Failed to open strength map: " + map;
        return false;
    }

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(bitmap), &bitmap);

    if (bitmap.bmHeight != STRENGTH_MAP_HEIGHT || bitmap.bmWidth != STRENGTH_MAP_WIDTH) {
#if DEBUG
        Logger::show("Invalid strength map size: "  + map);
#endif
        message = "Invalid strength map size: " + map;
        return false;
    }

    // Create a buffer to hold the pixel data
    std::vector<RGBQUAD> pixels(bitmap.bmWidth * bitmap.bmHeight);

    // Get the device context
    HDC hdc = GetDC(NULL);

    // Select the bitmap into the device context
    HBITMAP hOldBitmap = (HBITMAP) SelectObject(hdc, hBitmap);

    // Get the pixel data
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = bitmap.bmWidth;
    bmi.bmiHeader.biHeight = -bitmap.bmHeight; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // BGRA
    bmi.bmiHeader.biCompression = BI_RGB;
    GetDIBits(hdc, hBitmap, 0, bitmap.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS);

    // Revert the device context
    SelectObject(hdc, hOldBitmap);

    // Release the device context
    ReleaseDC(NULL, hdc);

    // Delete the bitmap
    DeleteObject(hBitmap);

    // Filter the pixels
    std::vector<RGBQUAD> filteredPixels;
    auto x = 0;
    auto y = 0;
    for (const RGBQUAD &pixel: pixels) {

        x++;
        if (x >= bitmap.bmWidth) {
            x = 0;
            y++;
        }
        auto px = pixel.rgbRed;
        strengthMap[x][y] = px;
        distribution[px]++;
    }

    auto minimums = distribution[0] + distribution[1] + distribution[2];
    auto maximums = distribution[255] + distribution[254] + distribution[253] + distribution[252] + distribution[251];

    auto size = STRENGTH_MAP_HEIGHT * STRENGTH_MAP_WIDTH;
    if (minimums >= (int) (size * 0.97f)) {
        // Too many zeros
        message = "Too many minimums" + std::to_string(minimums) + " max=" + std::to_string((int) (size * 0.99f));
        return false;
    }
    if (maximums >= (int) (size * 0.97f)) {
        // Too many maximums
        message = "Too many maximums: " + std::to_string(maximums) + " max=" + std::to_string((int) (size * 0.99f));
        return false;
    }
    if (maximums < 1) {
        // Too few maximums
        message = "White pixel (red>250) not found";
        return false;
    }
    message = split_string(map, '.').at(0);

    return true;
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
    if (auto brutePtr = dynamic_cast<ScreenshotProbeHashTableBrute *>(screenshotProbe.get())) {
        auto runningCache = is_running();
        set_running(false, true);
        std::vector<RGBQUAD> colors;
        std::string fname;
        if (read_next_colorconfig(colors, fname)) {
            Logger::show("Colors config: " + split_string(fname, '.').at(0));
            brutePtr->initialize_color_table(colors, true);
        } else {
            Logger::show("Can't toggle colorset");
        }
        if (runningCache) {
            set_running(runningCache, true);
        }
    }

}

void Manager::toggle_next_strengthmap() {
    auto runningCache = is_running();
    set_running(false, true);
    std::string message;
    if (read_next_strength_map(message)) {
        Logger::show("Strength map loaded: " + message);
        strength_map_ready = true;
    } else {
        Logger::show("Can't load strength map: " + message);
        strength_map_ready = false;
    }
    if (runningCache) {
        set_running(runningCache, true);
    }
}

void Manager::fill_multiplier_table() {
    int distance = 0;
    auto bracketSize = config.point_a_distance;
    for (auto i = 0; i < bracketSize; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp_value(float(distance) / float(bracketSize), config.multiplier_at_closest,
                                               config.multiplier_point_at_point_a);
    }
    bracketSize = config.point_a_b_distance;

    for (auto i = 0; i < bracketSize; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp_value(float(distance) / float(bracketSize), config.multiplier_point_at_point_a,
                                               config.multiplier_point_at_point_b);
    }
    bracketSize = config.point_b_c_distance;
    for (auto i = 0; i < 50; ++i) {
        distance++;
        if (distance > MULTIPLIER_TABLE_SIZE) break;
        multiplierTable[distance] = lerp_value(float(distance) / float(bracketSize), config.multiplier_point_at_point_b,
                                               config.multiplier_point_at_point_c);
    }
    for (auto i = distance; i < MULTIPLIER_TABLE_SIZE; ++i) {
        multiplierTable[i] = lerp_value((float) i / (MULTIPLIER_TABLE_SIZE -
                                                     (float) (config.point_a_distance + config.point_a_b_distance +
                                                              config.point_b_c_distance)),
                                        config.multiplier_point_at_point_c,
                                        config.multiplier_at_furthest);
    }
}

bool Manager::read_configuration(Configuration &config) {
    std::string line;
    std::ifstream configFile("MEMU3.config");
    if (configFile.is_open()) {
        while (getline(configFile, line)) {
            parse_config_file_line(config, line);
        }
        configFile.close();
        return true;
    }
    return false;
}


void Manager::save_config() const {
    std::ofstream configFileStream("MEMU3.config");

    configFileStream << "strength=" << config.strength << std::endl;
    configFileStream << "sensitivity=" << config.sensitivity << std::endl;
    configFileStream << "x_multiplier=" << config.x_multiplier << std::endl;
    configFileStream << "y_multiplier=" << config.y_multiplier << std::endl;

    // configFileStream << "point_a_distance=" << config.point_a_distance << std::endl;
    // configFileStream << "point_a_b_distance=" << config.point_a_b_distance << std::endl;
    // configFileStream << "point_b_c_distance=" << config.point_b_c_distance << std::endl;
    // configFileStream << "multiplier_at_closest=" << config.multiplier_at_closest << std::endl;
    // configFileStream << "multiplier_point_at_point_a=" << config.multiplier_point_at_point_a << std::endl;
    // configFileStream << "multiplier_point_at_point_b=" << config.multiplier_point_at_point_b << std::endl;
    // configFileStream << "multiplier_point_at_point_c=" << config.multiplier_point_at_point_c << std::endl;
    // configFileStream << "multiplier_at_furthest=" << config.multiplier_at_furthest << std::endl;

    configFileStream << "scan_horizontal_offset=" << config.scan_horizontal_offset << std::endl;
    configFileStream << "scan_vertical_offset=" << config.scan_vertical_offset << std::endl;
    configFileStream << "close_offset_x=" << config.close_offset_x << std::endl;
    configFileStream << "close_offset_y=" << config.close_offset_y << std::endl;
    configFileStream << "far_offset_x=" << config.far_offset_x << std::endl;
    configFileStream << "far_offset_y=" << config.far_offset_y << std::endl;
    configFileStream << "scan_width=" << config.scan_width << std::endl;
    configFileStream << "scan_height=" << config.scan_height << std::endl;

    configFileStream.close();
}

bool Manager::parse_config_file_line(Configuration &config, std::string &line) {
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
    } else if (key == "x_multiplier") {
        config.x_multiplier = (float) atof(value.c_str());
        return true;
    } else if (key == "y_multiplier") {
        config.y_multiplier = (float) atof(value.c_str());
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
    //} else if (key == "multiplier_at_closest") {
    //    config.multiplier_at_closest = (float) atof(value.c_str());
    //    return true;
    //} else if (key == "multiplier_point_at_point_a") {
    //    config.multiplier_point_at_point_a = (float)atof(value.c_str());
    //    return true;
    //} else if (key == "multiplier_point_at_point_b") {
    //    config.multiplier_point_at_point_b = (float)atof(value.c_str());
    //    return true;
    //} else if (key == "multiplier_point_at_point_c") {
    //    config.multiplier_point_at_point_c = (float)atof(value.c_str());
    //    return true;
    //} else if (key == "multiplier_at_furthest") {
    //    config.multiplier_at_furthest = (float)atof(value.c_str());
    //    return true;
    //} else if (key == "point_a_distance") {
    //    config.point_a_distance = atoi(value.c_str());
    //    return true;
    //} else if (key == "point_a_b_distance") {
    //    config.point_a_b_distance = atoi(value.c_str());
    //    return true;
    //} else if (key == "point_b_c_distance") {
    //    config.point_b_c_distance = atoi(value.c_str());
    //    return true;
    //}
    return false;
}
