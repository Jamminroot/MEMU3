#pragma once
#include "../Coords.h"
struct ProbeResult {
public:
    ProbeResult() = default;
    Coords coords = Coords();
    bool success;
};