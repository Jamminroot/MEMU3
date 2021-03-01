#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"

int main() {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    auto manager = Manager();
    Overlay::init(manager);
    auto assistant = AimAssistant(manager);
    manager.set_running(true);
    manager.stop_thread_until_exit();
}
