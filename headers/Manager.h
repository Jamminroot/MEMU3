#pragma once

#include "Rect.h"
#include "Coords.h"
#include "ScreenshotData.h"
#include <string>
#include <vector>

enum Mode {
    aim, trigger, flick, hanzo
};

class Manager {
public:
    static const int MULTIPLIER_TABLE_SIZE = 250;
    static const int COLOR_HASHTABLE_SIZE = (0xFFFFFF + 1) / 8;
    static const int MAXIMUM_TRIGGER_THRESHOLD_VALUE = 30;
    static inline const float MAXIMUM_AIM_STRENGTH_VALUE = 10.0f;
    static inline const float MAXIMUM_SENSITIVITY_VALUE = 25.0f;
    static const int MAXIMUM_HANZO_VERTICAL_OFFSET_VALUE = 35;
    void stop_thread_until_exit() const;
    void pause_thread_if_not_running() const;
    Manager(const Rect &pRegionSizeAndOffset, const Coords &pFarHeadOffset, const Coords &pCloseHeadOffset, const float &sensitivity, const float &pStrength);
    ~Manager();
    bool is_running() const;
    bool is_exit_requested() const;
    void set_running(const bool &state, const bool &silent = false);
    void request_exit();
    void toggle_mode();
    bool is_crosshair_over_enemy() const;
    void update_enemy_coords_with_local_coords(int x, int y);
    void update_enemy_coords_with_local_coords(Coords coords);
    void increase_mode_value();
    void decrease_mode_value();
    void decrease_sensitivity();
    void increase_sensitivity();
    void toggle_next_colorconfig();
    void initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile);
    bool enemyVisible = false;
    bool screenshotUpdatedAndEnemyVisible = false;
    bool flickReady = false;
    bool triggered = false;
    int triggerDistanceThreshold = 15;
    int hanzoVerticalOffset = 20;
    int mouseTriggerKeyStates = 0;
    int lastKnownIndex = 0;
    double elapsedScanTime = 0.0;
    float sensitivity;
    float strength;
    Mode mode = aim;
    Rect region;
    ScreenshotData screenshot;
    Coords enemyCoords;
    Coords screenSize;
    Coords lastKnownBarSize;
    BYTE colorHashTable[COLOR_HASHTABLE_SIZE]{0};
    float multiplierTable[MULTIPLIER_TABLE_SIZE]{0};
private:
    void fill_multiplier_table();
    static std::string hashtable_name(const std::vector<RGBQUAD> &pColors);
    bool dump_table(std::string &tablename) const;
    static bool probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor);
    bool restore_table(std::string &tablename) const;
    bool read_next_colorconfig(std::vector<RGBQUAD> &colors, std::string &config);
    std::vector<std::string> list_files_by_mask(const std::string &mask);
    static RGBQUAD parse_rgbquad_from_string(const std::string &line);
    int currentColorconfigIndex = 0;
    Coords median;
    Coords farHeadOffset;
    Coords closeHeadOffset;
    bool running;
    bool exitRequested;
};