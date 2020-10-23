#include "../headers/AimAssistant.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/Utils.h"
#include <thread>
#include <iostream>
#include <fstream>

#define COEFFICIENT_A 0.116f
#define IGNORED_BORDER_SIZE 5
#define SCANNING_THRESHOLD_PERCENT 80
#define CHECK_COEFFICIENT 100/(IGNORED_BORDER_SIZE*IGNORED_BORDER_SIZE)

AimAssistant::AimAssistant(class Manager &pManager, const float &sensitivity) : manager(pManager), input(manager) {
    modifier = sensitivity * COEFFICIENT_A;
    std::vector<RGBQUAD> colors = {
            {65,  38,  240, 34},
            {114, 81,  235, 15},
            {105, 70,  227, 15},
            {145, 124, 253, 15},
            {133, 99,  239, 15},
            {111, 99,  223, 15},
            {115, 103, 229, 15},
            {58,  54,  219, 15},
            {60,  58,  224, 15},
            {46,  23,  212, 15},
            {48,  41,  211, 15},
    };
    initialize_color_table(colors);
    std::thread mainThread(&AimAssistant::main_thread, this);
    SetThreadPriority(mainThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    mainThread.detach();
/*    std::thread inputThread(&AimAssistant::input_thread, this);
    SetThreadPriority(inputThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    inputThread.detach();*/
}

void AimAssistant::main_thread() {
    auto factory = ScreenshotFactory(manager);

    while (!manager.is_exit_requested()) {
        while (!manager.is_running()) {
           // std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        bool captured = factory.update_screenshot();
#if DEBUG
        auto start = std::chrono::high_resolution_clock::now();
#endif
        manager.enemyVisible = probe_healthbar_brute();

        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
        //std::cout << "Elapsed scan time: " << elapsed.count() << (manager.enemyVisible ? " (found at " : " (not found)");
        if (manager.enemyVisible) {
            find_healthbar_height();
            find_healthbar_width();
        }
        manager.screenshotHandled = false;
        handle_screenshot();
        //std::cout << std::endl;

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

static bool probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor) {
    auto dR = r - targetColor.rgbRed;
    auto dG = g - targetColor.rgbGreen;
    auto dB = b - targetColor.rgbBlue;
    auto checkResult = dR * dR + dG * dG + dB * dB <= targetColor.rgbReserved * targetColor.rgbReserved;
    return checkResult;
}

void AimAssistant::initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile) {
    //hashTable = BYTE[COLOR_HASHTABLE_SIZE];
    memset(hashTable, '\0', COLOR_HASHTABLE_SIZE);
    if (pUseCacheFile) {
        if (read_table()) {
            return;
        }
    }

    std::cout << "Building table.\n";
    int colorIndex = 0;
    for (auto targetColor : pColors) {
        colorIndex++;
        std::cout << "Color " << colorIndex << "/" << pColors.size();
        for (unsigned int i = 0x000000u; i <= 0xFFFFFFu; i++) {
            bool res = probe_bytes_against_rgbquad(((i & 0xFF0000) >> 16), ((i & 0x00FF00) >> 8), (i & 0x0000FF), targetColor);
            hashTable[i / 8] |= (byte) (res << (i % 8));
        }
        std::cout << " done.\n";
    }
#if DEBUG
    std::cout << "Checking colors against table.\n";
    for (auto targetColor : pColors) {
        std::cout << "Checking [" << (0xFF & targetColor.rgbRed) << "," << (0xFF & targetColor.rgbGreen) << "," << (0xFF & targetColor.rgbBlue) << "]: "
                  << probe_color(targetColor) << ".\n";
    }
#endif
    dump_table();
    //Manager.WriteByteArray("colorHashTable.bin", hashTable);
    //return hashTable;
}

bool AimAssistant::probe_color(const RGBQUAD &pColor) const {
    auto v = (pColor.rgbRed << 16) | (pColor.rgbGreen << 8) | pColor.rgbBlue;
    return (hashTable[v / 8] & ~(1 << (v % 8))) != 0;
}

bool AimAssistant::probe_healthbar_brute() const {
    auto index = coords_to_offset(manager.localEnemyCoords);
    auto regionWidth = manager.region.width;
    auto regionHeight = manager.region.height;

    if (probe_region_spiral(index, 9) && locate_healthbar_handle_left(index)) {
        manager.localEnemyCoords = offset_to_coords(index);
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
            manager.localEnemyCoords.set(x, y);
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

bool AimAssistant::dump_table() const {
    std::ofstream fout;
    fout.open("table.bin", std::ios::out | std::ios::binary);
    fout.write((const char *) hashTable, sizeof(hashTable));
    fout.close();
    return true;
}

bool AimAssistant::read_table() const {

    std::ifstream infile("table.bin");
    size_t chars_read;
    if (!(infile.read((char *) hashTable, sizeof(hashTable)))) // read up to the size of the buffer
    {
        if (!infile.eof()) {
            return false;
        }
    }
    chars_read = infile.gcount(); // get amount of characters really read.
    if (chars_read != sizeof(hashTable)) {
        return false;
    }
    return true;
}

void AimAssistant::find_healthbar_height() {
    auto offset = coords_to_offset(manager.localEnemyCoords);
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 20; ni++) {
        auto i = offset + ni;
        try {
            auto res = probe_color(manager.screenshot.data[i])
                       || probe_color(manager.screenshot.data[i + manager.region.width - 1])
                       || probe_color(manager.screenshot.data[i + manager.region.width * 2 - 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        }
        catch (_exception) {
        }
    }
    manager.lastKnownBarSize.x = check;
}

void AimAssistant::find_healthbar_width() {
    auto offset = coords_to_offset(manager.localEnemyCoords);
    auto check = 0;
    auto redsFound = false;
    for (auto ni = -5; ni < 15; ni++) {
        auto i = offset + manager.region.width * ni;
        try {
            auto res = probe_color(manager.screenshot.data[i])
                       || probe_color(manager.screenshot.data[i + 1])
                       || probe_color(manager.screenshot.data[i + 2]);
            if (!redsFound && res) redsFound = true;
            if (redsFound && !res) break;
            check += res;
        }
        catch (_exception) {
        }
    }
    manager.lastKnownBarSize.y = check;
}

void AimAssistant::input_thread() {
    while (!manager.is_exit_requested()) {
        while (!manager.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        while (manager.screenshotHandled || !manager.enemyVisible) {
           // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        handle_screenshot();
    }
}

void AimAssistant::aim_handler() {
    if (!manager.mouseTriggered) return;
    if (!manager.enemyVisible) return;
    std::thread moveThread(&AimAssistant::move_by, this, manager.relative_head_coords().x, manager.relative_head_coords().y);
    moveThread.detach();
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void AimAssistant::flick_handler() const {

}

void AimAssistant::hanzo_handler() const {

}

void AimAssistant::trigger_handler() const {

}

void AimAssistant::move_by(const int &pX, const int &pY) {
    terminate_threads();
    while (suspendThreads) {}//std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto x = pX;
    auto y = pY;
    threadCount++;
    apply_midifiers(x, y);
    x = (int) (x * modifier);
    y = (int) (y * modifier);
    auto steps = 5.000f;
    auto divX = (float) x * 1.0000f / (steps / 5.0000f );
    auto divY = (float) y * 1.0000f / (steps / 5.0000f);
    auto remainderX = 0.0f;
    auto remainderY = 0.0f;
    for (auto i = 0; i < steps; i++) {
        if (suspendThreads) {
            threadCount --;
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
}

void AimAssistant::terminate_threads() {
    suspendThreads = threadCount > 0;
}

void AimAssistant::apply_midifiers(int &x, int &y) const {
    float dirXMultiplier = 1;
    float dirYMultiplier = 1;
    if (x > 3) dirXMultiplier = 1;
    if (x < -3) dirXMultiplier = 1;
    if (y > 2) dirYMultiplier = 0.5f / 3;
    if (y < -2) dirYMultiplier = 0.5f / 3;
    const float min = 15.0f;
    const float max = 25.0f;
    auto mx = (clamp((float)abs(x), min, max) - min + 1) / (max - min);
    auto my = (clamp((float)abs(y), min, max) - min + 1) / (max - min);
    x = (int) ((float)x * mx * dirXMultiplier);
    y = (int) ((float)y * my * dirYMultiplier);
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
    manager.screenshotHandled = true;
}
