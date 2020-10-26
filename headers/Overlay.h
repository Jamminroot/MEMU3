#pragma once

#include "Manager.h"
#include "TtlStringCollection.h"
#include <string>

class Overlay {

public:
    static void init(Manager &pManager);
    static void show_hint(std::string msg, int timeout = 1000);
    static inline TtlStringCollection hints = TtlStringCollection(250);
    static void toggle_ui();
    static void toggle_debug_ui();
    static void toggle_render();
    RECT activeOverlayArea = RECT{0, 0, 250, 100};
private:
    static inline int renderEndTimePoint = (int) std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000;
    enum class DebugUiMode {
        Full, FrameOnly, TargetOnly, Off
    };

    enum class UiMode {
        Full, InfoOnly, DebugOnly, Off
    };
    void recalculate_active_overlay_area();
    void overlay_area_top_left_info();
    void overlay_area_bottom_right_info();
    void overlay_area_top_left_debug();
    void overlay_area_bottom_right_debug();

    static inline DebugUiMode debugUiMode = DebugUiMode::TargetOnly;
    static inline UiMode uiMode = UiMode::Full;
    void render_hints();
    void render_ui();
    void render_debug_ui();
    int init_d3d(HWND hWnd);
    int render();
    void render_info_clean();
    void gradient(int x, int y, int w, int h, int r, int g, int b, int a);
    void draw_center_line(float x, float y, int r, int g, int b, int a);
    void draw_line(float x, float y, float xx, float yy, int r, int g, int b, int a);
    void draw_filled(float x, float y, float w, float h, int r, int g, int b, int a);
    void draw_circle(float x, float y, float radius, int r, int g, int b, int a);
    void draw_box(float x, float y, float width, float height, float px, int r, int g, int b, int a);
    void draw_gui_box(float x, float y, float w, float h, int r, int g, int b, int a, int rr, int gg, int bb, int aa);

    int draw_string(std::string msg, int x, int y, int r, int g, int b, bool smallFont = false);
    int draw_string_shadow(char *String, int x, int y, int r, int g, int b);

    LRESULT CALLBACK callback_proc_instance(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK callback_proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

    Overlay(Manager &manager);
    ~Overlay();
    int WINAPI run();
    Manager &manager;
    static inline Overlay *sharedInstance;
    void draw_strength_ui(float y);
    void draw_sensitivity_ui(float y);
    void draw_hanzo_offset_ui(float y);
    void draw_trigger_threshold_ui(float y);
};
