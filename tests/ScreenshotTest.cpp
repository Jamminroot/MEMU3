#include "../headers/logging/console_logger.h"
#include "../headers/Utils.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"
#include "../headers/debug/debug_utils.h"
#include "../headers/probe/ScreenshotProbeColorPattern.h"

#include <string>
#include <iostream>
#include <chrono>
#include <filesystem>
#include "../headers/debug/scoped_time_meter.h"

ConsoleLogger l;

auto brute = ScreenshotProbeHashTableBrute();
auto vec = std::vector({7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
auto pattern = ScreenshotProbeColorPattern(vec, 80);
std::vector<COLORREF> colors = {RGB(255, 0, 0), RGB(50, 200, 50), RGB(0, 0, 255), RGB(0, 255, 255), RGB(255, 255, 0),
                                RGB(255, 0, 255)};
Rect r(400, 300, 0, 0);
ScreenshotFactory factory(r);
int check_image(const Rect &offset_region, const std::string &dir, const std::string &fname) {

    auto screenshot = ScreenshotData(offset_region);

    std::string full_path = dir + "\\" + fname;
    std::string dump_path = dir + "\\dump\\" + fname + "\\";

    if (std::filesystem::exists(dump_path)) {
        std::filesystem::remove_all(dump_path);
    }

    CreateDirectoryA((dir + "\\dump\\").c_str(), NULL);
    CreateDirectoryA(dump_path.c_str(), NULL);

    HBITMAP bitmap;

    load_image_offset_region(full_path, offset_region, bitmap);
    //print_hbitmap_console(bitmap);
    dump_bitmap(bitmap, dump_path + "_region.bmp");
    factory.update_screenshot_from_region_bitmap(screenshot, bitmap);
    std::vector<std::pair<std::string, std::vector<Coords>>> brute_layers;
    std::vector<std::pair<std::string, std::vector<Coords>>> pattern_layers;
    {
        ScopedTimeMeter("brute");
        brute.debug_probe_feature_layers(screenshot, brute_layers);
    }
    {
        ScopedTimeMeter("pattern");
        pattern.debug_probe_feature_layers(screenshot, pattern_layers);
    }
    BITMAPINFO bmi = create_bitmap_info_struct(offset_region.width, -offset_region.height, 24);

    VOID *pvBits;
    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP result = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);

/*    std::cout << "Brute matching:" << std::endl;
    for (int i = 0; i < brute_layers.size(); i++) {
        auto hint = pattern_layers[i].first;
        if (hint.find("Handle") == std::string::npos) {
            continue;
        }
        std::cout << "Layer " << i << " (" + brute_layers[i].first + ") size: " << brute_layers[i].second.size()
                  << "\n";
        debug_print_grey_background(result, bitmap, 0.95);
        debug_print_layer(brute_layers[i].first, brute_layers[i].second, result, colors[i % colors.size()]);
        dump_bitmap(result, dump_path + "brute_(" + std::to_string(i) +")" + brute_layers[i].first + ".bmp");
    }*/

    std::cout << "Pattern matching:" << std::endl;
    auto group_background_printed = false;
    // If folder exists - clear it


    auto join_groups = false;

    if (join_groups){


        bool inGroup = false;
        auto groups = 0;
        for (int i = 0; i < pattern_layers.size(); i++) {
            auto hint = pattern_layers[i].first;
            auto layer = pattern_layers[i].second;
            auto is_group = hint.find("group") != std::string::npos;

            if (!is_group || !group_background_printed) {
                debug_print_grey_background(result, bitmap, 0.95);
                if (is_group) {
                    group_background_printed = true;
                }
            }

            if (is_group) {
                groups++;
            }
            std::cout << "Layer " << i << " (" + hint + ") size: " << layer.size() << "\n";
            if (is_group) {
                debug_print_layer(hint, layer, result, colors[groups % colors.size()]);
            } else {
                debug_print_layer(hint, layer, result, colors[i % colors.size()]);
            }

            if (is_group) {
                inGroup = true;
            } else if (inGroup) {
                inGroup = false;
                dump_bitmap(result, dump_path + "pattern_(" + std::to_string(i) + ")_combined_groups("+std::to_string(groups)+").bmp");
            }

            if (!is_group || i == pattern_layers.size() - 1) {
                dump_bitmap(result, dump_path + "pattern_(" + std::to_string(i) + ")" + hint + ".bmp");
            }
        }
    } else {
        for (int i = 0; i < pattern_layers.size(); i++) {
            auto hint = pattern_layers[i].first;
            auto layer = pattern_layers[i].second;
            debug_print_grey_background(result, bitmap, 0.95);

            std::cout << "Layer " << i << " (" + hint + ") size: " << layer.size() << "\n";
           /* auto min_x = 999999;
            auto max_y = 0;

            for(auto c: layer){
                if (c.x < min_x){
                    min_x = c.x;
                }
                if (c.y > max_y){
                    max_y = c.y;
                }
            }
            std::cout << "min_x: " << min_x << " max_y: " << max_y << "\n";*/
            debug_print_layer(hint, layer, result, colors[i % colors.size()]);

            dump_bitmap(result, dump_path + "pattern_(" + std::to_string(i) + ")" + hint + ".bmp");

        }
    }

    DeleteObject(bitmap);
    DeleteObject(result);
    DeleteDC(hdc);

    auto start = std::chrono::high_resolution_clock::now();
    auto pattern_result = pattern.probe(screenshot);
    auto end = std::chrono::high_resolution_clock::now();
    auto pattern_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

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
/*        if (file.find("test_image (1)") == std::string::npos) {
            continue;
        }*/
        std::cout << "Checking " << file << std::endl;
        check_image(rect, dir, file);
        //break;
    }

    std::cout << "Done" << std::endl;
    return 0;
}