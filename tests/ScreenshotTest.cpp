#include "../headers/logging/console_logger.h"
#include "../headers/Utils.h"
#include "../headers/ScreenshotFactory.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"
#include "../headers/debug/debug_utils.h"

#include <string>
#include <iostream>
ConsoleLogger l;

auto brute = ScreenshotProbeHashTableBrute();
std::vector<COLORREF> colors = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(0, 255, 255), RGB(255, 255, 0), RGB(255, 0, 255)};

int check_image(const Rect &offset_region, const std::string &dir, const std::string &fname){
    auto screenshot = ScreenshotData(offset_region);

    std::string full_path = dir + "\\" + fname;
    std::string dump_path = dir + "\\dump\\" + fname + "\\";

    CreateDirectoryA((dir + "\\dump\\").c_str(), NULL);
    CreateDirectoryA(dump_path.c_str(), NULL);

    HBITMAP bitmap;

    load_image_offset_region(full_path, offset_region, bitmap);
    //print_hbitmap_console(bitmap);
    dump_bitmap(bitmap, dump_path + "_region.bmp");
    ScreenshotFactory::update_screenshot_from_region_bitmap(screenshot, bitmap );

    auto layers = brute.debug_probe_feature_layers(screenshot);

    BITMAPINFO bmi = create_bitmap_info_struct(offset_region.width, -offset_region.height, 24);

    VOID *pvBits;
    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP result = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    SelectObject(hdc, result);

    debug_print_grey_background(hdc, bitmap, 0.2);
    DeleteObject(bitmap);

    for(int i = 0; i < layers.size(); i++) {
        std::cout<<"Layer " << i << " size: " << layers[i].size() << "\n";
        debug_print_layer(layers[i], hdc, colors[i % colors.size()]);
        dump_bitmap(result, dump_path + "layer" + std::to_string(i) + ".bmp");
    }

    DeleteObject(result);
    DeleteDC(hdc);

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
    for(auto file: files) {
        check_image(rect, dir, file);
        break;
    }

    std::cout << "Done" << std::endl;
    return 0;
}