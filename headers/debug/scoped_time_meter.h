#pragma once

#include <string>
#include <chrono>
#include <utility>
#include <iostream>

class ScopedTimeMeter {
public:
    ScopedTimeMeter() {
        start = std::chrono::high_resolution_clock::now();
    }

    explicit ScopedTimeMeter(std::string name) : name(std::move(name)) {
        start = std::chrono::high_resolution_clock::now();
    }

    ~ScopedTimeMeter() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        Logger::show(name + " took " + std::to_string(duration) + "ns");
    }

private:
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};