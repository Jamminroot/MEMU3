#include <windows.h>
#include <winuser.h>

#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"
#include "../headers/logging/overlay_logger.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"

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

    auto settings = ProbeSettings();
    auto manager = Manager(std::make_unique<ScreenshotProbeHashTableBrute>(settings));
    OverlayLogger ol(manager);

    auto assistant = AimAssistant(manager);
    manager.set_running(true);
    manager.stop_thread_until_exit(hMutexHandle);
}
