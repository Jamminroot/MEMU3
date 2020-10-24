#pragma once

#include "../headers/Overlay.h"
#include "../headers/Utils.h"

#include <thread>
#include <iostream>

using namespace std;

HFONT *pHFont = new HFONT(CreateFont(22, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     DEFAULT_PITCH | FF_SWISS, L"Arial"));

std::vector<HWND> elements = std::vector<HWND>();

LRESULT CALLBACK Overlay::OverlayCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (WM_NCCREATE == message) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT *) lParam)->lpCreateParams);
        return TRUE;
    }
    return ((Overlay *) GetWindowLongPtr(hwnd, GWLP_USERDATA))->_OverlayCallback(hwnd, message, wParam, lParam);
}

void add_label(HWND &pParent, unsigned long id, int x, int y, int w, int h, std::string message) {
    string ids = std::to_string(id);
    //CreateWindowW(L"STATIC", selectedPatternProblem, WS_CHILD | WS_VISIBLE | SS_LEFT | ES_MULTILINE | WM_CTLCOLORSTATIC, 500, 190, 380, 90,*mainOverlayHwnd, (HMENU) 1, NULL, NULL);
    auto pHWnd = CreateWindow(L"static", s2ws(ids).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, y, w, h, pParent, (HMENU) id,
                              (HINSTANCE) GetWindowLong(pParent, -6), NULL);
    elements.push_back(pHWnd);
}

LRESULT CALLBACK Overlay::_OverlayCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC) wParam;

            DWORD CtrlID = GetDlgCtrlID((HWND) lParam); //Window Control ID

            SetTextColor(hdcStatic, RGB(157, 197, 206));
            SetBkColor(hdcStatic, RGB(40, 53, 79));
            return (LRESULT) hBrush;

        }

        case WM_DESTROY:
            //PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
};

int Overlay::Start() {
    RECT rc;
    HWND newhwnd = FindWindow(NULL, L"Untitled - Paint");
    while (newhwnd == NULL) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        newhwnd = FindWindow(NULL, L"Untitled - Paint");
    }
    GetWindowRect(newhwnd, &rc);
    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.hInstance = {0};
    wc.lpszClassName = L"MEMU-Overlay";
    wc.lpfnWndProc = OverlayCallback;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) RGB(0, 0, 0);
    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    if (!RegisterClass(&wc)) {
        return 0;
    }
    mainOverlayHwnd = CreateWindowW(L"MEMU-Overlay", L"MEMU Overlay", WS_EX_TOPMOST | WS_POPUP, rc.left, rc.top, 800, 800, HWND_DESKTOP, NULL, { 0 }, this);
    SetWindowLongW(mainOverlayHwnd, GWL_EXSTYLE, (int) GetWindowLong(mainOverlayHwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
    SetLayeredWindowAttributes(mainOverlayHwnd, 0, 255, LWA_ALPHA);

    ShowWindow(mainOverlayHwnd, SW_SHOWNORMAL);
    //ShowWindow(mainOverlayHwnd, SW_SHOW);
    MSG msg;
    ::SetWindowPos(FindWindow(NULL, L"Untitled - Paint"), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    while (manager.is_exit_requested()) {
        while (!FindWindowW(NULL, L"Untitled - Paint")) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        };
        ::SetWindowPos(mainOverlayHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        //if (topLeft != nullptr) topLeft->ClearCheck();

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //if (msg.message == WM_QUIT) exit(0);
        //
    }
}

Overlay::Overlay(Manager &pManager) : manager(pManager) {
    thread overlayThread(&Overlay::Start, this);
    overlayThread.detach();
    if (sharedInstance != nullptr) return;
    sharedInstance = this;
}

void Overlay::add_hint(std::string msg, int timeout) {
    if (sharedInstance == nullptr) return;
    sharedInstance->add_hint_instance(msg, timeout);
}

void Overlay::add_hint_instance(std::string msg, int timeout) {
    auto id = hints.add(msg, timeout);
    add_label(mainOverlayHwnd, id, 10, 20 * id, 100, 16, msg);
}

void Overlay::hint_deleted_callback(int id) {
    return sharedInstance->_hint_deleted_callback(id);
}

void Overlay::_hint_deleted_callback(int id) {
    std::cout << "CALLBACK! " << id << std::endl;
    int childId = -1;
    HWND handle;
    int index = elements.size() - 1;
    bool found = false;
    while (index >= 0 && !elements.empty()) {
        handle = elements.at(index);
        childId = GetDlgCtrlID(handle);
        if (id == childId) {

            found = true;
            break;
        }
        index--;
    }
    if (found) {
        elements.erase(elements.begin() + index);
        std::cout << "Deleted element" << id << std::endl;
        SendMessage(handle, WM_CLOSE, NULL, NULL);
    }
}
