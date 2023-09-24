#pragma once
#include <vector>
#include <string>

#include "../ScreenshotData.h"
#include "ProbeResult.h"
#include "../Coords.h"

const int IGNORED_BORDER_SIZE = 5;
const int CHECK_COEFFICIENT = 100 / (IGNORED_BORDER_SIZE * IGNORED_BORDER_SIZE);

class ScreenshotProbe {
public:
    ScreenshotProbe() = default;
    virtual ~ScreenshotProbe() = default;
    virtual bool probe(const ScreenshotData &data) = 0;
    virtual void debug_probe_feature_layers(const ScreenshotData &data, std::vector<std::pair<std::string, std::vector<Coords>>> &res) const = 0;
    ProbeResult get_probe_result() const { return probe_result; }
protected:
    Coords lastKnownBarSize;
    ProbeResult probe_result;

    virtual void find_healthbar_height(const ScreenshotData& screenshot) = 0;
    virtual void find_healthbar_width(const ScreenshotData& screenshot) = 0;
};