#include "../headers/AimAssistant.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/Utils.h"
#include "../headers/Overlay.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <random>
#include <mutex>
#include <condition_variable>

std::mutex terminate_thread_mutex;
std::mutex screenshot_mutex;
std::condition_variable terminate_thread_cond;
std::condition_variable screenshot_cond;

AimAssistant::AimAssistant(class Manager &pManager) : manager(pManager), input(manager) {
    std::vector<RGBQUAD> colors = {{65,  38,  240, 34},
                                   {114, 81,  235, 16},
                                   {105, 70,  227, 16},
                                   {145, 124, 253, 16},
                                   {133, 99,  239, 16},
                                   {111, 99,  223, 16},
                                   {115, 103, 229, 16},
                                   {58,  54,  219, 16},
                                   {60,  58,  224, 16},
                                   {46,  23,  212, 16},
                                   {48,  41,  211, 16},
                                   {0,   10,  221, 16}};
    initialize_color_table(colors);
    std::thread mainThread(&AimAssistant::main_thread, this);
    SetThreadPriority(mainThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    mainThread.detach();
    std::thread inputThread(&AimAssistant::input_thread, this);
    SetThreadPriority(inputThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    inputThread.detach();
}

void AimAssistant::main_thread() {
    auto factory = ScreenshotFactory(manager);

    while (!manager.is_exit_requested()) {
        manager.pause_thread_if_not_running();
        bool captured = factory.update_screenshot();
        if (!captured) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#if DEBUG
        auto start = std::chrono::high_resolution_clock::now();
#endif

        manager.enemyVisible = probe_healthbar_brute();
        if (manager.enemyVisible) {
            find_healthbar_height();
            find_healthbar_width();
        }
        std::unique_lock<std::mutex> lck(screenshot_mutex);
        manager.screenshotUpdatedAndEnemyVisible = manager.enemyVisible;
        screenshot_cond.notify_all();
    }
}

static bool probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor) {
    auto dR = r - targetColor.rgbRed;
    auto dG = g - targetColor.rgbGreen;
    auto dB = b - targetColor.rgbBlue;
    auto checkResult = dR * dR + dG * dG + dB * dB <= targetColor.rgbReserved * targetColor.rgbReserved;
    return checkResult;
}

std::string AimAssistant::hashtable_name(const std::vector<RGBQUAD> &pColors) {
    std::string bytes;
    for (auto targetColor : pColors) {
        bytes.push_back(targetColor.rgbRed);
        bytes.push_back(targetColor.rgbGreen);
        bytes.push_back(targetColor.rgbBlue);
        bytes.push_back(targetColor.rgbReserved);
    }
    return "ct_" + base64_encode(bytes) + ".bin";
}

void AimAssistant::initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile) {
    //hashTable = BYTE[COLOR_HASHTABLE_SIZE];
    memset(hashTable, '\0', COLOR_HASHTABLE_SIZE);
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
        for (unsigned int i = 0x000000u; i <= 0xFFFFFFu; i++) {
            bool res = probe_bytes_against_rgbquad(((BYTE) ((i & 0xFF0000) >> 16)), ((BYTE) ((i & 0x00FF00) >> 8)), (BYTE) (i & 0x0000FF), targetColor);
            hashTable[i / 8] |= (byte) (res << (i % 8));
        }
    }
#if DEBUG
    std::cout << "Checking colors against table.\n";
    for (auto targetColor : pColors) {
        std::cout << "Checking [" << (0xFF & targetColor.rgbRed) << "," << (0xFF & targetColor.rgbGreen) << "," << (0xFF & targetColor.rgbBlue) << "]: "
                  << probe_color(targetColor) << ".\n";
    }
#endif
    dump_table(tablename);
    //Manager.WriteByteArray("colorHashTable.bin", hashTable);
    //return hashTable;
}

bool AimAssistant::probe_color(const RGBQUAD &pColor) const {
    auto v = (pColor.rgbRed << 16) | (pColor.rgbGreen << 8) | pColor.rgbBlue;
    return (hashTable[v / 8] & ~(1 << (v % 8))) != 0;
}

bool AimAssistant::probe_healthbar_brute() const {
    auto index = manager.lastKnownIndex;
    auto regionWidth = manager.region.width;
    auto regionHeight = manager.region.height;

    if (probe_region_spiral(index, 9) && locate_healthbar_handle_left(index)) {
        manager.update_enemy_coords_with_local_coords(offset_to_coords(index));
        manager.lastKnownIndex = index;
        return true;
    }

    for (auto x = IGNORED_BORDER_SIZE + 2; x < regionWidth - IGNORED_BORDER_SIZE - 2; x++)
        for (auto y = IGNORED_BORDER_SIZE + 2; y < regionHeight - IGNORED_BORDER_SIZE - 2; y++) {
            auto i = coords_to_offset(x, y);
            if (!probe_color(manager.screenshot.data[i])) continue;
            auto xx = x;
            while (xx > IGNORED_BORDER_SIZE && (probe_all_points_diagonal(i - 1) || probe_any_point_left(i - 3))) {
                xx--;
                i = coords_to_offset(xx, y);
            }
            if (!probe_handle(i)) continue;
            manager.update_enemy_coords_with_local_coords(x, y);
            manager.lastKnownIndex = i;
            return true;
        }
    return false;
}

bool AimAssistant::probe_region_spiral(int &offset, const int side) const {
    auto startCoords = offset_to_coords(offset);
    auto regionWidth = manager.region.width;
    auto regionHeight = manager.region.height;

    // Possibly swap region width and height places
    if (abs(min(startCoords.x, regionHeight - startCoords.x) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
    if (abs(min(startCoords.y, regionWidth - startCoords.y) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
    if (probe_color(manager.screenshot.data[offset])) return true;
    auto directionSign = -1;

    // First 2 steps are hard-coded (outside of for-loops)
    offset += regionWidth;
    if (probe_color(manager.screenshot.data[offset])) return true;
    offset++;
    if (probe_color(manager.screenshot.data[offset])) return true;

    for (auto currentMaxSideSize = 2; currentMaxSideSize <= side; currentMaxSideSize++) {
        for (auto currentSideStep = 1; currentSideStep <= currentMaxSideSize; currentSideStep++) {
            offset += regionWidth * directionSign;
            if (offset < 0 || offset >= manager.screenshot.size) return false;
            if (!probe_color(manager.screenshot.data[offset])) continue;
            return true;
        }

        for (auto currentSideStep = 1; currentSideStep <= currentMaxSideSize; currentSideStep++) {
            offset += directionSign;
            if (offset < 0 || offset >= manager.screenshot.size) return false;
            if (!probe_color(manager.screenshot.data[offset])) continue;
            return true;
        }
        directionSign = -directionSign;
    }
    return false;
}

bool AimAssistant::locate_healthbar_handle_left(int &offset) const {
    auto inputX = offset % manager.region.width;
    auto inputY = offset / manager.region.width;
    auto found = false;
    for (auto xi = inputX; xi > IGNORED_BORDER_SIZE + 2; xi--) {
        auto newOffset = coords_to_offset(xi, inputY);
        if (!probe_handle(newOffset)) continue;
        offset = newOffset;
        found = true;
    }
    return found;
}

Coords AimAssistant::offset_to_coords(const int &offset) const {
    Coords coords = Coords();
    coords.x = offset % manager.region.width;
    coords.y = offset / manager.region.width;
    return coords;
}

int AimAssistant::coords_to_offset(const Coords &coords) const {
    return coords.x + coords.y * manager.region.width;
}

int AimAssistant::coords_to_offset(const int &pX, const int &pY) const {
    return pX + pY * manager.region.width;
}

bool AimAssistant::probe_all_points_diagonal(const int &offset, const int pLineSize) const {
    auto check = 0;
    for (auto ni = 0; ni < pLineSize; ni++) {
        auto i = offset + (manager.region.width * ni - ni);
        auto res = probe_color(manager.screenshot.data[i]);
        check += res;
    }
    return check / CHECK_COEFFICIENT >= SCANNING_THRESHOLD_PERCENT;
}

bool AimAssistant::probe_any_point_left(const int &offset, const int pLineSize) const {
    for (auto ni = 0; ni < pLineSize; ni++) {
        auto i = offset + (manager.region.width * ni - ni);
        if (probe_color(manager.screenshot.data[i]))
            return true;
    }
    return false;
}

bool AimAssistant::probe_handle(const int &index) const {
    auto successfulChecks = 0;
    for (auto nx = 0; nx < IGNORED_BORDER_SIZE; nx++)
        for (auto ny = 0; ny < IGNORED_BORDER_SIZE; ny++) {
            // Y axis is inverted (starts at bottom-left); Also healthbar is slash-oriented (`/`), thus X is adjusted by neighbor-Y arrayOffset
            auto i = index - ny * manager.region.width + nx - ny;
            successfulChecks += probe_color(manager.screenshot.data[i]);
        }

    return successfulChecks * CHECK_COEFFICIENT >= SCANNING_THRESHOLD_PERCENT;
}

bool AimAssistant::dump_table(std::string &tablename) const {
    std::ofstream outStream;
    outStream.open(tablename, std::ios::out | std::ios::binary);
    outStream.write((const char *) hashTable, sizeof(hashTable));
    outStream.close();
    Overlay::show_hint("Saving color scan table to file.");
    return true;
}

bool AimAssistant::restore_table(std::string &tablename) const {
    Overlay::show_hint("Restoring cached color scan table.");
    std::ifstream inStream(tablename);
    size_t chars_read;
    if (!(inStream.read((char *) hashTable, sizeof(hashTable)))) {
        if (!inStream.eof()) {
            return false;
        }
    }
    chars_read = inStream.gcount();
    return chars_read == sizeof(hashTable);
}

void AimAssistant::find_healthbar_height() {
    auto offset = manager.lastKnownIndex;
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 20; ni++) {
        auto i = offset + ni;
        try {
            auto res = probe_color(manager.screenshot.data[i]) || probe_color(manager.screenshot.data[i + manager.region.width - 1]) ||
                       probe_color(manager.screenshot.data[i + manager.region.width * 2 - 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        } catch (_exception) {
        }
    }
    manager.lastKnownBarSize.x = check;
}

void AimAssistant::find_healthbar_width() {
    auto offset = manager.lastKnownIndex;
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 15; ni++) {
        auto i = offset + manager.region.width * ni;
        try {
            auto res = probe_color(manager.screenshot.data[i]) || probe_color(manager.screenshot.data[i + 1]) || probe_color(manager.screenshot.data[i + 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        } catch (_exception) {
        }
    }
    manager.lastKnownBarSize.y = check;
}

void AimAssistant::input_thread() {
    while (!manager.is_exit_requested()) {
        manager.pause_thread_if_not_running();
        std::unique_lock<std::mutex> lck(screenshot_mutex);
        if (!manager.screenshotUpdatedAndEnemyVisible) screenshot_cond.wait(lck);
        handle_screenshot();
        manager.screenshotUpdatedAndEnemyVisible = false;
    }
}

void AimAssistant::aim_handler() {
    if (!manager.triggered) return;
    if (!manager.enemyVisible) return;
    std::thread moveThread(&AimAssistant::move_by_smoothed, this, manager.enemyCoords);
    moveThread.detach();
}

void AimAssistant::flick_handler() {
    if (!manager.triggered) return;
    if (!manager.enemyVisible) return;
    std::thread flickThread(&AimAssistant::flick_and_shot, this, manager.enemyCoords);
    flickThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay() * 5));
}

void AimAssistant::hanzo_handler() {
    if (!manager.triggered) return;
    if (!manager.flickReady) return;
    if (!manager.enemyVisible) return;
    std::thread flickThread(&AimAssistant::flick_and_release, this, manager.enemyCoords);
    flickThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay() * 15));
}

void AimAssistant::trigger_handler() const {
    if (!manager.enemyVisible) return;
    if (!manager.is_crosshair_over_enemy()) return;
    input.lmb_click();
}

void AimAssistant::flick_and_shot(const Coords &coords) {
    terminate_threads();
    std::unique_lock<std::mutex> lck(terminate_thread_mutex);
    if (suspendThreads) terminate_thread_cond.wait(lck);
    auto target = coords;
    threadCount++;
    apply_modifiers_common(target);
    input.move_by(target.x, target.y);
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay()));
    input.lmb_click();
    suspendThreads = false;
    threadCount--;
    terminate_thread_cond.notify_all();
}

void AimAssistant::flick_and_release(const Coords &coords) {
    terminate_threads();
    std::unique_lock<std::mutex> lck(terminate_thread_mutex);
    if (suspendThreads) terminate_thread_cond.wait(lck);
    auto target = coords;
    threadCount++;
    apply_modifiers_common(target);
    input.move_by(target.x, target.y);
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay()));
    input.lmb_release();
    suspendThreads = false;
    threadCount--;
    terminate_thread_cond.notify_all();
}

void AimAssistant::move_by_smoothed(const Coords &coords) {
    terminate_threads();
    std::unique_lock<std::mutex> lck(terminate_thread_mutex);
    if (suspendThreads) terminate_thread_cond.wait(lck);
    auto target = coords;
    threadCount++;
    apply_modifiers_common(target);
    apply_modifiers_distance(target);
    apply_modifiers_strength(target);

    auto steps = 5.000f;
    auto divX = (float) target.x / (steps);
    auto divY = (float) target.y / (steps);
    auto remainderX = 0.0f;
    auto remainderY = 0.0f;
    for (auto i = 0; i < steps; i++) {
        if (suspendThreads) {
            threadCount--;
            suspendThreads = false;
            return;
        }
        remainderX += divX;
        remainderY += divY;
        input.move_by((int) ceil(remainderX), (int) ceil(remainderY));
        remainderX -= ceil(remainderX);
        remainderY -= ceil(remainderY);
    }
    suspendThreads = false;
    threadCount--;
    terminate_thread_cond.notify_all();
}

void AimAssistant::terminate_threads() {
    suspendThreads = threadCount > 0;
}

void AimAssistant::apply_modifiers_common(Coords &coords) const {
    coords.x = (int) ((float) coords.x * manager.sensitivity);
    coords.y = (int) ((float) coords.y * manager.sensitivity);
}

void AimAssistant::apply_modifiers_hanzo(Coords &coords) {
    coords.y -= 15;
}

void AimAssistant::handle_screenshot() {
    switch (manager.mode) {
        case aim:
            aim_handler();
            break;
        case flick:
            if (manager.flickReady) {
                flick_handler();
            } else {
                aim_handler();
            }
            break;
        case hanzo:
            hanzo_handler();
            break;
        case trigger:
            trigger_handler();
            break;
        default:
            break;
    }
}

void AimAssistant::apply_modifiers_distance(Coords &coords) const {
    float distanceBasedMultiplier;
    if (coords.length <= 25.0f) {
        distanceBasedMultiplier = lerp(coords.length / 25.0f, 0.2f, 0.5f);
    } else if (coords.length <= 50) {
        distanceBasedMultiplier = lerp((coords.length - 25.0f) / 25.0f, 0.5f, 1.0f);
    } else if (coords.length <= 100) {
        distanceBasedMultiplier = lerp((coords.length - 50.0f) / 50.0f, 1.0f, 0.3f);
    } else {
        distanceBasedMultiplier = 0.25f;
    }
    coords.x = (int) ((float) coords.x * distanceBasedMultiplier);
    coords.y = (int) ((float) coords.y * distanceBasedMultiplier);
}

void AimAssistant::apply_modifiers_strength(Coords &coords) const {
    coords.x = (int) ((float) coords.x * (manager.strength / 10.0f));
    coords.y = (int) ((float) coords.y * (manager.strength / 35.0f));
}
