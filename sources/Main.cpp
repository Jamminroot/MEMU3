#include <windows.h>
#include <winuser.h>

#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"

#pragma comment(lib,"user32.lib")

void HideConsole()
{
    FreeConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

int main() {
    HANDLE hMutexHandle = CreateMutex(NULL, true, L"MEMU3");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 0;
    }
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    HideConsole();
    auto manager = Manager();
    Overlay::init(manager);
    auto assistant = AimAssistant(manager);
    manager.set_running(true);
    manager.stop_thread_until_exit(hMutexHandle);
}
