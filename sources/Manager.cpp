#include "../headers/Manager.h"
#include "../headers/Utils.h"

Manager::~Manager() {
}

bool Manager::is_running() const {
    return running;
}

void Manager::set_running(const bool &state) {
    running = state;
}

Manager::Manager(const int width, const int height, const int offsetLeft, const int offsetTop, const Coords &pFarHeadOffset, const Coords &pCloseHeadOffset)
        : running(false), exitRequested(false), screenshot(ScreenshotData(width, height)), farHeadOffset(pFarHeadOffset), closeHeadOffset(pCloseHeadOffset) {
    RECT desktop;
    const auto hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    screenSize.x = desktop.right;
    screenSize.y = desktop.bottom;
    int left = screenSize.x / 2 + offsetLeft;
    int top = screenSize.y / 2 + offsetTop;
    region = Rect(width, height, left, top);
    median.x = region.left - screenSize.x / 2;
    median.y = region.top + region.height - screenSize.y / 2;
    enemyCoords = Coords();
}

bool Manager::is_exit_requested() const {
    return exitRequested;
}

void Manager::request_exit() {
    running = false;
    exitRequested = true;
}

void Manager::update_enemy_coords_with_local_coords(int x, int y) {
    auto dv = (clamp(((float) lastKnownBarSize.x / 2.0f + (float) lastKnownBarSize.y), 5.0f, 15.0f) - 5.0f) / 10.0f;
    enemyCoords.set((int) lerp(dv, (float) closeHeadOffset.x, (float) farHeadOffset.x) + median.x + x,
                    (int) lerp(dv, (float) closeHeadOffset.y, (float) farHeadOffset.y) + median.y - y);
}

bool Manager::is_crosshair_over_enemy() const {
    return false;
}

void Manager::update_enemy_coords_with_local_coords(Coords coords) {
    update_enemy_coords_with_local_coords(coords.x, coords.y);
}
