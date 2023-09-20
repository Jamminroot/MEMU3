#pragma once

#include "ScreenshotProbe.h"
#include <vector>
#include <string>
#include <fstream>
#include "../Utils.h"
#include "../logging/logger.h"

static const unsigned int COLOR_HASHTABLE_SIZE = (0xFFFFFF + 1) / 8;

class ScreenshotProbeHashTable : public ScreenshotProbe {
public:
    ScreenshotProbeHashTable();

    void initialize_color_table(const std::vector<RGBQUAD> &pColors, const bool pUseCacheFile);

    bool probe_color(const RGBQUAD &color) const;

protected:
    const int SCANNING_THRESHOLD_PERCENT = 83;

    bool probe_bytes_against_rgbquad(const BYTE r, const BYTE g, const BYTE b, const RGBQUAD targetColor) const;

    bool dump_table(std::string &tablename) const;

    bool probe_all_points_diagonal(const ScreenshotData &screenshot, const int &offset, const int pLineSize = 5) const;

    bool probe_any_point_left(const ScreenshotData &screenshot, const int &offset, const int pLineSize = 5) const;

    bool restore_table(std::string &tablename) const;

    void find_healthbar_height(const ScreenshotData &screenshot) override;

    void find_healthbar_width(const ScreenshotData &screenshot) override;

    bool locate_healthbar_handle_left(const ScreenshotData &screenshot, int &offset) const;

    bool probe_handle(const ScreenshotData &screenshot, const int &index) const;

private:

    BYTE hashTable[COLOR_HASHTABLE_SIZE]{0};
};
