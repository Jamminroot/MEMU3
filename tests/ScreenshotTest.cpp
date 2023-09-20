#include "../headers/logging/console_logger.h"
#include "../headers/Utils.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"
#include "../headers/debug/debug_utils.h"

#include <string>
#include <iostream>
ConsoleLogger l;

auto brute = ScreenshotProbeHashTableBrute();


int check_image(ScreenshotData &screenshot, const std::string &file, bool dbg = false){

    Rect region = screenshot.region;

    HBITMAP bitmap;

    load_image_offset_region(file, region, bitmap);
    print_hbitmap_console(bitmap);
    dump_bitmap(bitmap, L"test.bmp");
    ScreenshotFactory::update_screenshot_from_region_bitmap(screenshot, bitmap );

    DeleteObject(bitmap);
    auto layers = brute.debug_probe_feature_layers(screenshot);

    if (brute.probe(screenshot)) {
        std::cout<<"Brute: " << brute.get_probe_result().coords.x << ", " << brute.get_probe_result().coords.y << "\n";
    } else {
        std::cout<<"Brute: not found\n";
    }
    return 0;
}

int main(int c, char** args) {
    std::cout<< "Starting..." << std::endl;
    if (c<2) {
        std::cout << "Usage: " << args[0] << " <path to folder>" << std::endl;
        return 1;
    }
    auto dir = std::string(args[1]);

    //Set current directory
    if(!SetCurrentDirectoryA(dir.c_str())) {
        std::cerr << "Could not set current directory\n";
        return 1;
    }

    // list all files in folder
    auto files = list_files_by_mask(".jpg", dir);
    auto rect = Rect(400, 300, -200, -200);


    auto screenshot = ScreenshotData(rect);
    for(auto file: files) {
        std::string full_path = dir + "\\" + file;
        check_image(screenshot, full_path, true);
        break;
    }

    std::cout << "Done" << std::endl;
    return 0;
}