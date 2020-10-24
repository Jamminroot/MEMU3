#pragma once
#include "Manager.h"
#include <d3dx9.h>

class Overlay {
public:
    static LRESULT CALLBACK Proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK _Proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
    static void Init(HINSTANCE hInstance, Manager& pManager);

private:
    Overlay(Manager &manager);
    int WINAPI Run(HINSTANCE hInstance);
    int D3D9Init(HWND hWnd);
    int Render();
    void GradientFunc(int x, int y, int w, int h, int r, int g, int b, int a);
    void DrawCenterLine(float x, float y, int r, int g, int b, int a);
    void DrawLine(float x, float y, float xx, float yy, int r, int g, int b, int a);
    void DrawFilled(float x, float y, float w, float h, int r, int g, int b, int a);
    void DrawBox(float x, float y, float width, float height, float px, int r, int g, int b, int a);
    void DrawGUIBox(float x, float y, float w, float h, int r, int g, int b, int a, int rr, int gg, int bb, int aa);
    void DrawHealthBar(float x, float y, float w, float h, int r, int g, int b, int a);
    void DrawHealthBarBack(float x, float y, float w, float h, int a);

    int DrawString(char *String, int x, int y, int r, int g, int b, ID3DXFont *ifont);
    int DrawShadowString(char *String, int x, int y, int r, int g, int b, ID3DXFont *ifont);
    Manager &manager;
    static inline Overlay *sharedInstance;
};
