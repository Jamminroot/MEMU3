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
    static const int MAXIMUM_TRIGGER_THRESHOLD_VALUE = 30;
    static inline const float MAXIMUM_AIM_STRENGTH_VALUE = 10.0f;
    static inline const float MAXIMUM_SENSITIVITY_VALUE = 25.0f;
    static const int MAXIMUM_HANZO_VERTICAL_OFFSET_VALUE = 35;
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
    void increase_mode_value();
    void decrease_mode_value();
    void decrease_sensitivity();
    void increase_sensitivity();
    float sensitivity;
    std::atomic_bool enemyVisible;
    bool screenshotUpdatedAndEnemyVisible;
    int triggerDistanceThreshold = 15;
    int hanzoVerticalOffset = 20;
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