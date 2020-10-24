#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"
#include <iostream>
#include <thread>

#if DEBUG

void TerminateIn30Seconds(Manager &manager) {
    std::this_thread::sleep_for(std::chrono::seconds(30));
    manager.request_exit();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(0);
}

#endif

HINSTANCE hInstance;

int main() {
    std::cout << "Starting\n";
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    auto manager = Manager(400, 300, -200, -200, Coords(50, 35), Coords(65, 65));
    auto assistant = AimAssistant(manager, 3.5f);
    manager.set_running(true);
    Overlay::Init(hInstance, manager);
    /*std::thread olt(overlayInit);
    olt.detach();*/
    /*  Overlay overlay = Overlay(manager);
      Overlay::add_hint("Hello", 1000);
      Overlay::add_hint("Hello - 2", 1000 * 2);
      Overlay::add_hint("Hello - 3", 1000 * 3);
      Overlay::add_hint("Hello - 3", 1000 * 4);*/

    /* std::thread overlayThread(&::OverlayThreadHandler, overlay);
     SetThreadPriority(overlayThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
     overlayThread.detach();*/



/*#if DEBUG
    std::thread terminator(TerminateIn30Seconds, manager);
    terminator.detach();
#endif*/

    while (!manager.is_exit_requested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
