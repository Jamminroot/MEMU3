#include "../headers/Manager.h"
#include "../headers/Utils.h"

#include <iostream>

Manager::~Manager() {
}

bool Manager::is_running() const {
    return running;
}

void Manager::set_running(const bool &state) {
    running = state;
}

Manager::Manager(const int width,
                 const int height,
                 const int offsetLeft,
                 const int offsetTop,
                 const Coords &pFarHeadOffset,
                 const Coords &pCloseHeadOffset) : running(false),
                                                   exitRequested(false),
                                                   screenshot(ScreenshotData(width, height)),
                                                   farHeadOffset(pFarHeadOffset),
                                                   closeHeadOffset(pCloseHeadOffset) {
    RECT desktop;
    const auto hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    screenSize.x = desktop.right;
    screenSize.y = desktop.bottom;
    int left = screenSize.x / 2 + offsetLeft;
    int top = screenSize.y / 2 + offsetTop;
    std::cout<<"Left: "<<left <<" Top: "<<top <<std::endl;
    region = Rect(width, height, left, top);
    median.x =  region.left - screenSize.x / 2; // -screenSize.x / 2 + region.left;
    median.y = region.top + region.height - screenSize.y / 2 ;//screenSize.y / 2 - region.top - region.height;
    localEnemyCoords = Coords();
}

bool Manager::is_exit_requested() const {
    return exitRequested;
}

void Manager::request_exit() {
    running = false;
    exitRequested = true;
}

Coords Manager::relative_head_coords() const {
   /*
    return Coords((int) lerp(dv, (float) closeHeadOffset.x, (float) farHeadOffset.x) + screenSize.x / 2- (region.left + localEnemyCoords.x),
                  (int) lerp(dv, (float) closeHeadOffset.y, (float) farHeadOffset.y) + screenSize.y / 2- (region.top + region.height - localEnemyCoords.y));*/
     auto dv = (clamp(((float) lastKnownBarSize.x / 2.0f + (float) lastKnownBarSize.y), 5.0f, 15.0f) - 5.0f) / 10.0f;
     return Coords((int)  lerp(dv, (float) closeHeadOffset.x, (float) farHeadOffset.x) + median.x + localEnemyCoords.x,
                   (int)  lerp(dv, (float) closeHeadOffset.y, (float) farHeadOffset.y) + median.y - localEnemyCoords.y)  ;
}
