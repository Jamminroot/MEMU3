#include "../headers/AimAssistant.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/Utils.h"
#include "../headers/logging/logger.h"
#include <thread>
#include <random>
#include <mutex>
#include <condition_variable>

std::mutex terminate_thread_mutex;
std::mutex screenshot_mutex;
std::condition_variable terminate_thread_cond;
std::condition_variable screenshot_cond;

AimAssistant::AimAssistant(class Manager &pManager) : manager(pManager), input(manager) {
    manager.toggle_next_strengthmap();
    std::thread mainThread(&AimAssistant::main_thread, this);
    SetThreadPriority(mainThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    mainThread.detach();
    std::thread inputThread(&AimAssistant::input_thread, this);
    SetThreadPriority(inputThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
    inputThread.detach();
}

void AimAssistant::main_thread() {
    auto factory = ScreenshotFactory(manager.region);
    ScreenshotData screenshot(manager.region);
    while (!manager.is_exit_requested()) {
        manager.pause_thread_if_not_running();
        bool captured = factory.update_screenshot_data(screenshot);
        if (!captured) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        auto start = std::chrono::high_resolution_clock::now();
        manager.enemyVisible = manager.screenshotProbe->probe(screenshot);
        if (manager.enemyVisible) {
            manager.update_enemy_coords_with_local_coords(manager.screenshotProbe->get_probe_result().coords);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
        manager.elapsedScanTime = elapsed.count();
        std::unique_lock<std::mutex> lck(screenshot_mutex);
        manager.screenshotUpdatedAndEnemyVisible = manager.enemyVisible;
        screenshot_cond.notify_all();
    }
}

void AimAssistant::input_thread() {
    while (!manager.is_exit_requested()) {
        manager.pause_thread_if_not_running();
        std::unique_lock<std::mutex> lck(screenshot_mutex);
        if (!manager.screenshotUpdatedAndEnemyVisible) screenshot_cond.wait(lck);
        input_handler();
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
    apply_modifiers_sensitivity(target);
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
    apply_modifiers_hanzo(target);
    apply_modifiers_sensitivity(target);
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
    if (manager.strength_map_ready){
        apply_modifiers_strength_map(target);
    }
    apply_modifiers_strength(target);
    //apply_modifiers_distance(target);
    apply_modifiers_sensitivity(target);


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

void AimAssistant::apply_modifiers_sensitivity(Coords &coords) const {
    coords.x = (int) ((float) coords.x * manager.config.sensitivity);
    coords.y = (int) ((float) coords.y * manager.config.sensitivity);}

void AimAssistant::apply_modifiers_hanzo(Coords &coords) {
    coords.y -= 15;
}

void AimAssistant::input_handler() {
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

// That is outdated, and only here for reference
void AimAssistant::apply_modifiers_distance(Coords &coords) const {
    auto index = (int) coords.vector_length;
    float distance_multiplier;
    if (index >= Manager::MULTIPLIER_TABLE_SIZE) {
        distance_multiplier = 0.2f;
    } else {
        distance_multiplier = manager.multiplierTable[index];
    }
    coords.x = (int) ((float) coords.x * distance_multiplier);
    coords.y = (int) ((float) coords.y * distance_multiplier);
}

void AimAssistant::apply_modifiers_strength_map(Coords &coords) const {

    BYTE b = manager.strengthMap[coords.x + manager.STRENGTH_MAP_WIDTH/2][coords.y + manager.STRENGTH_MAP_HEIGHT / 2];

    auto str = static_cast<float>(b) / 255.0f;
    coords.x = (int) ((float) coords.x * str);
    coords.y = (int) ((float) coords.y * str);
}

void AimAssistant::apply_modifiers_strength(Coords &coords) const {
    coords.x = (int) ((float) coords.x * manager.config.strength * manager.config.x_multiplier);
    coords.y = (int) ((float) coords.y * manager.config.strength * manager.config.y_multiplier);
}

