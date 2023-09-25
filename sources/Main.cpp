#include <windows.h>
#include <winuser.h>

#include "../headers/Manager.h"
#include "../headers/AimAssistant.h"
#include "../headers/Overlay.h"
#include "../headers/logging/overlay_logger.h"
#include "../headers/probe/ScreenshotProbeHashTableBrute.h"
#include "../headers/probe/ScreenshotProbeColorPattern.h"
#include "../headers/logging/console_logger.h"

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
    auto vec = std::vector({7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17});
    auto pattern = ScreenshotProbeColorPattern(vec, 80);
    auto manager = Manager(std::make_unique<ScreenshotProbeColorPattern>(pattern));
    OverlayLogger ol(manager);

    auto assistant = AimAssistant(manager);
    manager.set_running(true);
    manager.stop_thread_until_exit(hMutexHandle);
}
