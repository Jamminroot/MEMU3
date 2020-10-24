#pragma once

#include "Rect.h"
#include "Coords.h"
#include "ScreenshotData.h"
#include <string>
#include <atomic>

enum aim_mode {
    aim, trigger, flick, hanzo
};

class Manager {
public:
    Manager(const int width, const int height, const int left, const int top, const Coords &pFarHeadOffset, const Coords &pCloseHeadOffset);
    ~Manager();
    bool is_running() const;
    bool is_exit_requested() const;
    void set_running(const bool &);
    void request_exit();
    std::string targetWindowName = "Overwatch";
    std::atomic_bool enemyVisible;
    std::atomic_bool screenshotHandled;

    bool flickReady;
    aim_mode mode = aim;
    int mouseTriggerKeyStates = 0;
    bool mouseTriggered;
    int triggerStateChanged;
    bool is_crosshair_over_enemy() const;
    Coords enemyCoords;
    int lastKnownIndex = 0;
    void update_enemy_coords_with_local_coords(int x, int y);
    void update_enemy_coords_with_local_coords(Coords coords);
    Coords lastKnownBarSize;
    Rect region;
    ScreenshotData screenshot;
    Coords screenSize;
    Coords median;
    Coords farHeadOffset;
    Coords closeHeadOffset;
private:
    bool running;
    bool exitRequested;
};