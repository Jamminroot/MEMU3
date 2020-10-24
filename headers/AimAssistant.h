#pragma once

#include "../headers/Manager.h"
#include "../headers/InputController.h"
#include "../headers/Coords.h"
#include <vector>
#include <atomic>

#define COLOR_HASHTABLE_SIZE (0xFFFFFF + 1) / 8

class AimAssistant {
public:
    AimAssistant(class Manager &pManager, const float &sensitivity = 3.5f);
private:
    BYTE hashTable[COLOR_HASHTABLE_SIZE];
    Manager &manager;
    InputController input;
    float modifier;
    std::atomic_bool suspendThreads = false;
    std::atomic_int threadCount = 0;
    void main_thread();
    void input_thread();
    void handle_screenshot();
    bool probe_region_spiral(int &offset, const int side = 25) const;
    bool locate_healthbar_handle_left(int &offset) const;
    bool probe_healthbar_brute() const;
    void initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile = true);
    bool probe_color(const RGBQUAD &pColor) const;
    bool dump_table() const;
    bool read_table() const;
    Coords offset_to_coords(const int &offset) const;
    int coords_to_offset(const Coords &coords) const;
    int coords_to_offset(const int &pX, const int &pY) const;
    bool probe_all_points_diagonal(const int &, const int = 5) const;
    bool probe_any_point_left(const int &, const int = 5) const;
    bool probe_handle(const int &) const;
    void find_healthbar_height();
    void find_healthbar_width();
    void move_by(const int &x, const int &y);
    void aim_handler();
    void trigger_handler() const;
    void flick_handler() const;
    void hanzo_handler() const;
    void terminate_threads();
    void apply_midifiers(int &x, int &y) const;
};