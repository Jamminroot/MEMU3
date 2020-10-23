#pragma once

#include "Manager.h"
#include "TtlStringCollection.h"

class Overlay {
public:
    static void add_hint(std::string msg, int timeout = 1000);

    void add_hint_instance(std::string msg, int timeout = 1000);
    Overlay(Manager &manager);
private:
    HWND mainOverlayHwnd;
    static inline HBRUSH hBrush = CreateSolidBrush(RGB(40, 53, 79));
    LRESULT CALLBACK _OverlayCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK OverlayCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    int Start();

    static inline Overlay *sharedInstance = nullptr;
    Manager &manager;
    static void hint_deleted_callback(int id);
    void _hint_deleted_callback(int id);
    TtlStringCollection hints = TtlStringCollection(1000, hint_deleted_callback);
};