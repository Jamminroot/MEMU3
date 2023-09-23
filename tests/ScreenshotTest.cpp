#include "../headers/logging/console_logger.h"
#include "../headers/Utils.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"
#include "../headers/debug/debug_utils.h"
#include "../headers/probe/ScreenshotProbeColorPattern.h"

#include <string>
#include <iostream>
#include <chrono>
#include "../headers/debug/scoped_time_meter.h"

ConsoleLogger l;

auto brute = ScreenshotProbeHashTableBrute();
auto vec = std::vector({7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
auto pattern = ScreenshotProbeColorPattern(vec, 80);
std::vector<COLORREF> colors = {RGB(255, 0, 0), RGB(50, 200, 50), RGB(0, 0, 255), RGB(0, 255, 255), RGB(255, 255, 0),
                                RGB(255, 0, 255)};

int check_image(const Rect &offset_region, const std::string &dir, const std::string &fname) {
    auto screenshot = ScreenshotData(offset_region);

    std::string full_path = dir + "\\" + fname;
    std::string dump_path = dir + "\\dump\\" + fname + "\\";

    CreateDirectoryA((dir + "\\dump\\").c_str(), NULL);
    CreateDirectoryA(dump_path.c_str(), NULL);

    HBITMAP bitmap;

    load_image_offset_region(full_path, offset_region, bitmap);
    //print_hbitmap_console(bitmap);
    dump_bitmap(bitmap, dump_path + "_region.bmp");
    ScreenshotFactory::update_screenshot_from_region_bitmap(screenshot, bitmap);
    auto brute_layers = std::vector<std::pair<std::string, std::vector<Coords>>>();
    auto pattern_layers = std::vector<std::pair<std::string, std::vector<Coords>>>();
    {
        ScopedTimeMeter("brute");
        brute_layers = brute.debug_probe_feature_layers(screenshot);
    }
    {
        ScopedTimeMeter("pattern");
        pattern_layers = pattern.debug_probe_feature_layers(screenshot);
    }
    BITMAPINFO bmi = create_bitmap_info_struct(offset_region.width, -offset_region.height, 24);

    VOID *pvBits;
    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP result = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);

    std::cout << "Brute matching:" << std::endl;
    for (int i = 0; i < brute_layers.size(); i++) {
        std::cout << "Layer " << i << " (" + brute_layers[i].first + ") size: " << brute_layers[i].second.size()
                  << "\n";
        debug_print_grey_background(result, bitmap, 0.95);
        debug_print_layer(brute_layers[i].first, brute_layers[i].second, result, colors[i % colors.size()]);
        dump_bitmap(result, dump_path + "brute_(" + std::to_string(i) +")" + brute_layers[i].first + ".bmp");
    }

    std::cout << "Pattern matching:" << std::endl;
    for (int i = 0; i < pattern_layers.size(); i++) {
        std::cout << "Layer " << i << " (" + pattern_layers[i].first + ") size: " << pattern_layers[i].second.size()
                  << "\n";
        debug_print_grey_background(result, bitmap, 0.95);
        debug_print_layer(pattern_layers[i].first, pattern_layers[i].second, result, colors[i % colors.size()]);
        dump_bitmap(result, dump_path + "pattern_(" + std::to_string(i) + ")"+ pattern_layers[i].first +  + ".bmp");
    }

    DeleteObject(bitmap);
    DeleteObject(result);
    DeleteDC(hdc);

    // measure time
    auto start = std::chrono::high_resolution_clock::now();
    auto brute_result = brute.probe(screenshot);
    auto end = std::chrono::high_resolution_clock::now();
    auto brute_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();


    start = std::chrono::high_resolution_clock::now();
    auto pattern_result = pattern.probe(screenshot);
    end = std::chrono::high_resolution_clock::now();
    auto pattern_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    if (brute_result) {
        std::cout << "Brute: " << brute.get_probe_result().coords.x << ", " << brute.get_probe_result().coords.y
                  << "\n";
    } else {
        std::cout << "Brute: not found\n";
    }
    std::cout << "Brute time: " << brute_time << " ns\n";

    if (pattern_result) {
        std::cout << "Pattern: " << pattern.get_probe_result().coords.x << ", " << pattern.get_probe_result().coords.y
                  << "\n";
    } else {
        std::cout << "Pattern: not found\n";
    }
    std::cout << "Pattern time: " << pattern_time << " ns\n";
    return 0;
}


int main(int c, char **args) {
    std::cout << "Starting..." << std::endl;
    if (c < 2) {
        std::cout << "Usage: " << args[0] << " <path to folder>" << std::endl;
        return 1;
    }
    auto dir = std::string(args[1]);

    //Set current directory
    if (!SetCurrentDirectoryA(dir.c_str())) {
        std::cerr << "Could not set current directory\n";
        return 1;
    }

    // list all files in folder
    auto files = list_files_by_mask(".jpg", dir);
    auto rect = Rect(400, 300, -200, -200);
    for (auto file: files) {
        std::cout << "Checking " << file << std::endl;
        check_image(rect, dir, file);
        //break;
    }

    std::cout << "Done" << std::endl;
    return 0;
}