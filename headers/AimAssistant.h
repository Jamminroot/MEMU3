#pragma once

#include "../headers/Manager.h"
#include "../headers/InputController.h"
#include "../headers/Coords.h"
#include <vector>
#include <atomic>

class AimAssistant {

public:
    AimAssistant(class Manager &pManager);
private:
    Manager &manager;
    InputController input;
    std::atomic_bool suspendThreads = false;
    std::atomic_int threadCount = 0;
    void main_thread();
    void input_thread();
    void input_handler();
    void move_by_smoothed(const Coords &coords);
    void flick_and_shot(const Coords &coords);
    void flick_and_release(const Coords &coords);
    void aim_handler();
    void trigger_handler() const;
    void flick_handler();
    void hanzo_handler();
    void terminate_threads();
    void apply_modifiers_sensitivity(Coords &coords) const;
    static void apply_modifiers_hanzo(Coords &coords);
    void apply_modifiers_distance(Coords &coords) const;
    void apply_modifiers_strength(Coords &coords) const;
    void apply_modifiers_strength_map(Coords &coords) const;
};