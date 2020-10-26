#pragma once

#include "Manager.h"
#include "TtlStringCollection.h"

class Overlay {

public:
    static void init(Manager &pManager);
    static void show_hint(std::string msg, int timeout = 1000);
    static inline TtlStringCollection hints = TtlStringCollection(250);
    static void toggle_ui();
    static void toggle_debug_ui();
private:

    enum class DebugUiMode {
        Full, FrameOnly, TargetOnly, Off
    };

    enum class UiMode {
        Full, InfoOnly, DebugOnly, Off
    };

    static inline DebugUiMode debugUiMode = DebugUiMode::TargetOnly;
    static inline UiMode uiMode = UiMode::Full;
    void render_hints();
    void render_ui();
    void render_debug_ui();
    int init_d3d(HWND hWnd);
    int render();
    void gradient(int x, int y, int w, int h, int r, int g, int b, int a);
    void draw_center_line(float x, float y, int r, int g, int b, int a);
    void draw_line(float x, float y, float xx, float yy, int r, int g, int b, int a);
    void draw_filled(float x, float y, float w, float h, int r, int g, int b, int a);
    void draw_circle(float x, float y, float radius, int r, int g, int b, int a);
    void draw_box(float x, float y, float width, float height, float px, int r, int g, int b, int a);
    void draw_gui_box(float x, float y, float w, float h, int r, int g, int b, int a, int rr, int gg, int bb, int aa);
    void draw_healthbar(float x, float y, float w, float h, int r, int g, int b, int a);
    void draw_healthbar_back(float x, float y, float w, float h, int a);

    int draw_string(char *String, int x, int y, int r, int g, int b);
    int draw_string_shadow(char *String, int x, int y, int r, int g, int b);

    LRESULT CALLBACK callback_proc_instance(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK callback_proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

    Overlay(Manager &manager);
    ~Overlay();
    int WINAPI run();

    Manager &manager;
    static inline Overlay *sharedInstance;
};
