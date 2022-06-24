#include "../headers/AimAssistant.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/Utils.h"
#include <thread>
#include <random>
#include <mutex>
#include <condition_variable>

std::mutex terminate_thread_mutex;
std::mutex screenshot_mutex;
std::condition_variable terminate_thread_cond;
std::condition_variable screenshot_cond;

AimAssistant::AimAssistant(class Manager &pManager) : manager(pManager), input(manager), hashTable(pManager.colorHashTable) {
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
                                   {0,   10,  221, 16},};
    manager.initialize_color_table(colors, true);
    colors.clear();
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
        auto start = std::chrono::high_resolution_clock::now();
        manager.enemyVisible = probe_healthbar_brute();
        if (manager.enemyVisible) {
            find_healthbar_height();
            find_healthbar_width();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
        manager.elapsedScanTime = elapsed.count();
        std::unique_lock<std::mutex> lck(screenshot_mutex);
        manager.screenshotUpdatedAndEnemyVisible = manager.enemyVisible;
        screenshot_cond.notify_all();
    }
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
    auto i = (IGNORED_BORDER_SIZE) + IGNORED_BORDER_SIZE * regionWidth;

    // For Y we perform a reversed iteration - that is to address 0:0 being in _bottom_-left corner
    for (auto y = regionHeight - IGNORED_BORDER_SIZE; y > IGNORED_BORDER_SIZE; --y) {
        for (auto x = IGNORED_BORDER_SIZE; x < regionWidth - IGNORED_BORDER_SIZE; ++x) {
            // Next i - vertically. Unchecked, since we should not exceed that with the loop bounds above.
            i += 1;
            //auto i = coords_to_offset(x, y);
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
        // Reset to the beginning of a new line - "0" x (actually + 0+border_size) plus height*width
        i = (IGNORED_BORDER_SIZE) + (y - 1) * regionWidth;
    }
    return false;
}

bool AimAssistant::probe_region_spiral(int &offset, const int side) const {
    auto startCoords = offset_to_coords(offset);
    auto regionWidth = manager.region.width;
    auto regionHeight = manager.region.height;

    // Possibly swap region width and height places
    if (fabs(fmin(startCoords.x, regionHeight - startCoords.x) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
    if (fabs(fmin(startCoords.y, regionWidth - startCoords.y) - (side / 2 + 1)) <= IGNORED_BORDER_SIZE + 2) return false;
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
        } catch (...) {
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
        } catch (...) {
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
    manager.readyForNextFlick = false;
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
    apply_modifiers_hanzo(target);
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
            if (manager.triggered && manager.flickReady && manager.readyForNextFlick) {
                flick_handler();
            } else if (!manager.flickReady) {
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
    auto index = (int) coords.length;
    float distance_multiplier;
    if (index >= Manager::MULTIPLIER_TABLE_SIZE) {
        distance_multiplier = 0.2f;
    } else {
        distance_multiplier = manager.multiplierTable[index];
    }
    coords.x = (int) ((float) coords.x * distance_multiplier);
    coords.y = (int) ((float) coords.y * distance_multiplier);
}

void AimAssistant::apply_modifiers_strength(Coords &coords) const {
    coords.x = (int) ((float) coords.x * manager.strength * manager.x_multiplier);
    coords.y = (int) ((float) coords.y * manager.strength * manager.y_multiplier);
}

