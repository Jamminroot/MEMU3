#pragma once

#include "Configuration.h"
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
    static const unsigned int MULTIPLIER_TABLE_SIZE = 512;
    static const unsigned int COLOR_HASHTABLE_SIZE = (0xFFFFFF + 1) / 8;
    static const unsigned int MAXIMUM_TRIGGER_THRESHOLD_VALUE = 30;
    static inline const float MAXIMUM_AIM_STRENGTH_VALUE = 10.0f;
    static inline const float MAXIMUM_SENSITIVITY_VALUE = 25.0f;
    static const int MAXIMUM_HANZO_VERTICAL_OFFSET_VALUE = 35;
    static const int STRENGTH_MAP_HEIGHT = 600;
    static const int STRENGTH_MAP_WIDTH = 800;

    void stop_thread_until_exit(HANDLE& handle) const;
    void pause_thread_if_not_running() const;
    Manager();
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
    void toggle_next_strengthmap();
    void initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile);
    bool enemyVisible = false;
    bool screenshotUpdatedAndEnemyVisible = false;
    bool flickReady = false;
    bool readyForNextFlick = true;
    bool triggered = false;
    int triggerDistanceThreshold = 15;
    int hanzoVerticalOffset = 20;
    int mouseTriggerKeyStates = 0;
    int lastKnownIndex = 0;
    int scan_vertical_offset = 0;
    int scan_horizontal_offset = 0;
    double elapsedScanTime = 0.0;
    float sensitivity;
    float strength;
    float x_multiplier;
    float y_multiplier;

    float multiplier_at_closest=0.01f;
    float multiplier_point_at_point_a=0.4f;
    float multiplier_point_at_point_b=1.0f;
    float multiplier_point_at_point_c=0.8f;
    float multiplier_at_furthest=0.2f;

    int point_a_distance = 25;
    int point_a_b_distance = 25;
    int point_b_c_distance = 50;

    Mode mode = aim;
    Rect region;
    ScreenshotData screenshot;
    Coords enemyCoords;
    Coords screenSize;
    Coords lastKnownBarSize;
    BYTE colorHashTable[COLOR_HASHTABLE_SIZE]{0};
    BYTE strengthMap[STRENGTH_MAP_WIDTH][STRENGTH_MAP_HEIGHT]{0};
    float multiplierTable[MULTIPLIER_TABLE_SIZE]{0};
    bool strength_map_ready;
private:
    void save_config() const;
    bool parse_config_file_line(Configuration &config, std::string &line);
    Configuration read_configuration();

    void fill_multiplier_table();
    static std::string hashtable_name(const std::vector<RGBQUAD> &pColors);
    bool dump_table(std::string &tablename) const;
    static bool probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor);
    bool restore_table(std::string &tablename) const;
    bool read_next_colorconfig(std::vector<RGBQUAD> &colors, std::string &config);
    bool read_next_strength_map(std::string &map);
    std::vector<std::string> list_files_by_mask(const std::string &mask);
    static RGBQUAD parse_rgbquad_from_string(const std::string &line);
    int currentColorconfigIndex = 0;
    int currentStrengthMapIndex = 0;
    Coords median;
    Coords farHeadOffset;
    Coords closeHeadOffset;
    bool running;
    bool exitRequested;
};