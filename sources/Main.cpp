#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"

int main() {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    auto manager = Manager(Rect(400, 300, -200, -200), Coords(50, 60), Coords(65, 65), 3.5f, 3.0f);
    Overlay::init(manager);
    auto assistant = AimAssistant(manager);
    manager.set_running(true);
    Overlay::show_hint("MEMU Started", 5000);
    manager.stop_thread_until_exit();
}
