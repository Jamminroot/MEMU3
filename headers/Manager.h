#pragma once

#include "Rect.h"
#include "Coords.h"
#include "ScreenshotData.h"
#include <string>
#include <atomic>

enum Mode {
    aim, trigger, flick, hanzo
};

class Manager {
public:
    void stop_thread_until_exit() const;
    void pause_thread_if_not_running() const;
    Manager(const int width, const int height, const int left, const int top, const Coords &pFarHeadOffset, const Coords &pCloseHeadOffset, const float &sensitivity, const float &pStrength);
    ~Manager();
    bool is_running() const;
    bool is_exit_requested() const;
    void set_running(const bool &);
    void request_exit();
    void toggle_mode();
    bool is_crosshair_over_enemy() const;
    void update_enemy_coords_with_local_coords(int x, int y);
    void update_enemy_coords_with_local_coords(Coords coords);
    void increase_aim_strength();
    void decrease_aim_strength();
    void decrease_sensitivity();
    void increase_sensitivity();
    float sensitivity;
    std::atomic_bool enemyVisible;
    bool screenshotUpdatedAndEnemyVisible;
    bool flickReady;
    float strength;
    Mode mode = aim;
    int mouseTriggerKeyStates = 0;
    bool triggered;
    Coords enemyCoords;
    int lastKnownIndex = 0;
    Rect region;
    ScreenshotData screenshot;
    Coords screenSize;
    Coords lastKnownBarSize;
private:
    Coords median;
    Coords farHeadOffset;
    Coords closeHeadOffset;
    bool running;
    bool exitRequested;
};