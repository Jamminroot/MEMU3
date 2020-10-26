#pragma once

#include "../headers/Overlay.h"
#include "../headers/Utils.h"

#include <algorithm>
#include <thread>
#include <iostream>
#include <dwmapi.h>
#include <d3d9.h>
#include <d3dx9core.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

HWND hWnd;//, TargetWnd;
MSG Message;

const MARGINS pMargin = {0, 0, 2560, 1440};

IDirect3D9Ex *dx_Object = nullptr;
IDirect3DDevice9Ex *dx_Device = nullptr;
D3DPRESENT_PARAMETERS dx_Params;
ID3DXLine *dx_Line = nullptr;
ID3DXSprite *dx_Sprite = nullptr;
IDirect3DTexture9 *dx_AimTexture = nullptr;
IDirect3DTexture9 *dx_FlickTexture = nullptr;
IDirect3DTexture9 *dx_HanzoTexture = nullptr;
IDirect3DTexture9 *dx_TriggerTexture = nullptr;
ID3DXFont *dx_Font = nullptr;

LPDIRECT3DTEXTURE9 load_texture(std::string filename, D3DCOLOR transcolor) {
    LPDIRECT3DTEXTURE9 texture = NULL;

    //get width and height from bitmap file
    D3DXIMAGE_INFO info;
    HRESULT result = D3DXGetImageInfoFromFile(s2ws(filename).c_str(), &info);
    if (result != D3D_OK) {
        return NULL;
    }

    //create the new textur by loading a bitmap image file
    result = D3DXCreateTextureFromFileEx(dx_Device, s2ws(filename).c_str(), info.Width, info.Height, 1, D3DPOOL_DEFAULT, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
                                         D3DX_DEFAULT, D3DX_DEFAULT, transcolor, &info, NULL, &texture);

    //make sure the bitmap texture was loaded correctly
    if (result != D3D_OK) {
        return NULL;
    }

    return texture;
}

void
sprite_transform_draw(LPDIRECT3DTEXTURE9 image, int x, int y, int width, int height, int frame, int columns, float rotation, float scaling, D3DCOLOR color) {
    //Create a scale vector
    D3DXVECTOR2 scale(scaling, scaling);

    //Create a translate vector
    D3DXVECTOR2 trans((float) x, (float) y);

    //Set center by dividing width and height by two
    D3DXVECTOR2 center((float) (width * scaling) / 2, (float) (height * scaling) / 2);

    //Create 2D transformation matrix
    D3DXMATRIX mat;
    D3DXMatrixTransformation2D(&mat, NULL, 0, &scale, &center, rotation, &trans);

    //Tell sprite object to use the transform
    dx_Sprite->SetTransform(&mat);

    //Calculate frame location in source image
    int fx = (frame % columns) * width;
    int fy = (frame / columns) * height;
    RECT srcRect = {fx, fy, fx + width, fy + height};

    //draw the sprite frame
    dx_Sprite->Draw(image, &srcRect, NULL, NULL, color);
}

int Overlay::draw_string(char *String, int x, int y, int r, int g, int b) {
    RECT ShadowPos;
    ShadowPos.left = x + 1;
    ShadowPos.top = y + 1;
    RECT FontPos;
    FontPos.left = x;
    FontPos.top = y;
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
    return 0;
}

int Overlay::draw_string_shadow(char *String, int x, int y, int r, int g, int b) {
    RECT Font;
    Font.left = x;
    Font.top = y;
    RECT Fonts;
    Fonts.left = x + 1;
    Fonts.top = y;
    RECT Fonts1;
    Fonts1.left = x - 1;
    Fonts1.top = y;
    RECT Fonts2;
    Fonts2.left = x;
    Fonts2.top = y + 1;
    RECT Fonts3;
    Fonts3.left = x;
    Fonts3.top = y - 1;
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &Fonts3, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &Fonts2, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &Fonts1, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &Fonts, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    dx_Font->DrawTextA(nullptr, String, (int) strlen(String), &Font, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
    return 0;
}

void Overlay::gradient(int x, int y, int w, int h, int r, int g, int b, int a) {
    int iColorr, iColorg, iColorb;
    for (int i = 1; i < h; i++) {
        iColorr = (int) ((float) i / h * r);
        iColorg = (int) ((float) i / h * g);
        iColorb = (int) ((float) i / h * b);
        draw_filled((float) x, (float) y + i, (float) w, (float) 1, r - iColorr, g - iColorg, b - iColorb, a);
    }
}

void Overlay::draw_line(float x, float y, float xx, float yy, int r, int g, int b, int a) {
    D3DXVECTOR2 dLine[2];

    dx_Line->SetWidth(1);

    dLine[0].x = x;
    dLine[0].y = y;

    dLine[1].x = xx;
    dLine[1].y = yy;

    dx_Line->Draw(dLine, 2, D3DCOLOR_ARGB(a, r, g, b));

}

void Overlay::draw_circle(float x, float y, float radius, int r, int g, int b, int alpha) {
    D3DXVECTOR2 Line[128];
    float Step = 3.14159265f * 2.0f / 50.0f;
    int Count = 0;
    for (float a = 0; a < 3.14159265 * 2.0; a += Step) {
        float X1 = radius * cos(a) + x;
        float Y1 = radius * sin(a) + y;
        float X2 = radius * cos(a + Step) + x;
        float Y2 = radius * sin(a + Step) + y;
        Line[Count].x = X1;
        Line[Count].y = Y1;
        Line[Count + 1].x = X2;
        Line[Count + 1].y = Y2;
        Count += 2;
    }
    dx_Line->SetWidth(1);
    dx_Line->Draw(Line, Count, D3DCOLOR_ARGB(alpha, r, g, b));
}

void Overlay::draw_filled(float x, float y, float w, float h, int r, int g, int b, int a) {
    D3DXVECTOR2 vLine[2];

    dx_Line->SetWidth(w);

    vLine[0].x = x + w / 2;
    vLine[0].y = y;
    vLine[1].x = x + w / 2;
    vLine[1].y = y + h;

    dx_Line->Begin();
    dx_Line->Draw(vLine, 2, D3DCOLOR_RGBA(r, g, b, a));
    dx_Line->End();
}

void Overlay::draw_box(float x, float y, float width, float height, float px, int r, int g, int b, int a) {
    D3DXVECTOR2 points[5];
    points[0] = D3DXVECTOR2(x, y);
    points[1] = D3DXVECTOR2(x + width, y);
    points[2] = D3DXVECTOR2(x + width, y + height);
    points[3] = D3DXVECTOR2(x, y + height);
    points[4] = D3DXVECTOR2(x, y);
    dx_Line->SetWidth(px);
    dx_Line->Draw(points, 5, D3DCOLOR_RGBA(r, g, b, a));
}

void Overlay::draw_gui_box(float x, float y, float w, float h, int r, int g, int b, int a, int rr, int gg, int bb, int aa) {
    draw_box(x, y, w, h, 1, r, g, b, a);
    draw_filled(x, y, w, h, rr, gg, bb, aa);
}

void Overlay::draw_healthbar(float x, float y, float w, float h, int r, int g, int b, int a) {
    draw_filled(x, y, w, h, r, g, b, a);
}

void Overlay::draw_healthbar_back(float x, float y, float w, float h, int a) {
    draw_filled(x, y, w, h, 0, 0, 0, a);
}

void Overlay::draw_center_line(float x, float y, int width, int r, int g, int b) {
    D3DXVECTOR2 dPoints[2];
    dPoints[0] = D3DXVECTOR2(x, y);
    dPoints[1] = D3DXVECTOR2((float) manager.screenSize.x / 2, (float) manager.screenSize.y);
    dx_Line->SetWidth((float) width);
    dx_Line->Draw(dPoints, 2, D3DCOLOR_RGBA(r, g, b, 255));
}

/*
We require to initialize the D3D drawing, so we require hWnd. Windows identifies each form or application by assigning it a handle or also known as hWnd.
*/
int Overlay::init_d3d(HWND pHWnd) {
    // We get our Process Access and Module Bases


    /*
    We need to check to see if we can create an IDirect3D9Ex object and return an interface to it. Why is D3D_SDK_VERSION passed? Because we will need to ensure that the header files used in the compiled application match the version of the installed runtime DLLs. Why are we passing the object to dx_Object? Because we are creating an IDirect3D9Ex object, and we need to store it somewhere. If it fails, the app crashes (the DLL), and if it passes, it continues, simple huh?
    */
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &dx_Object))) {
        return 1;
    }

    /*
    We created the dx_Param earlier, it is a D3DPRESENT_PARAMETERS structure. It contains many variables you can modify but in this source we are only modifying these variables.

    BackBufferFormat (D3DFORMAT) is the buffer that is drawn off-screen and will be switched with the front buffer at the next frame. This is considered double buffering, which is what you need to do in GDI to ensure that it does not flicker. But GDI will still flicker because it is "slow" you could persay.

    D3DFMT_A8R8G8B8	(Value: 21) is an 32-bit ARGB pixel format with alpha, using 8 bits per channel.
    */

    dx_Params.BackBufferFormat = D3DFMT_A8R8G8B8;

    dx_Params.BackBufferWidth = manager.screenSize.x;
    dx_Params.BackBufferHeight = manager.screenSize.y;
    dx_Params.EnableAutoDepthStencil = TRUE;
    dx_Params.AutoDepthStencilFormat = D3DFMT_D16;

    /*
    hDeviceWindow (HWND) is the form or application that determines the location and size of the back buffer on the screen.
    */
    dx_Params.hDeviceWindow = pHWnd;

    /*
    MultiSampleQuality (DWORD) is the quality level. Technically speaking DEFAULT_QUALITY is zero which also is kind of funny because zero is the lowest MultiSampleQuality. Why are we setting this? Well this is all GPU related, and microsoft is extremely vauge about this, so we will just leave this as zero.
    */
    dx_Params.MultiSampleQuality = DEFAULT_QUALITY;

    /*
    SwapEffect (D3DSWAPEFFECT) is how the front and back buffer are to be swapped. When we disregard this, we can do multi sampling (above).
    */
    dx_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;

    /*
    Windowed (BOOL) is basically asking you if the form or application is running windowed or fullscreen. True is windowed. False is fullscreen.
    */
    dx_Params.Windowed = TRUE;


    /*
    We need to see if we can create a device to REPRESENT the display adapter.
    D3DADAPTER_DEFAULT (UNIT) is the always the primary display adapter.
    D3DDEVTYPE_HAL (D3DDEVTYPE) (value: 1) is hardware rasterization. Shading is done with software, hardware, or mixed transform and lighting.
    hWnd (HWND) is the form we will create the device in mind with. Something I noticed was that this can be null if we set the hDeviceWindow to a non-null value, which it is. So I changed it to a null.
    D3DCREATE_HARDWARE_VERTEXPROCESSING (DWORD) specifies hardware vertex processing.
    dx_Param (D3DPRESENT_PARAMTERS*) describe the presentation parameters for the device to be created.
    0 (D3DDISPLAYMODEEX*) is only used when the device is set to fullscreen, which it is not for this source, but it basically is the properties of a display mode (size, width, height, refresh rate, format, scanlineordering).
    dx_Device (IDirect3DDevice9Ex**) is the address of a pointer to the returned IDirect3DDevice9Ex, which represents the created device.
    */
    if (FAILED(dx_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &dx_Params, 0, &dx_Device))) {
        return 1;
    }

    if (!dx_Line)
        D3DXCreateLine(dx_Device, &dx_Line);
    if (!dx_Sprite)
        D3DXCreateSprite(dx_Device, &dx_Sprite);
    if (!dx_AimTexture && dx_Sprite)
        dx_AimTexture = load_texture("Aim.bmp", D3DCOLOR_XRGB(73, 125, 65));
    if (!dx_FlickTexture && dx_Sprite)
        dx_FlickTexture = load_texture("Flick.bmp", D3DCOLOR_XRGB(73, 125, 65));
    if (!dx_HanzoTexture && dx_Sprite)
        dx_HanzoTexture = load_texture("Hanzo.bmp", D3DCOLOR_XRGB(73, 125, 65));
    if (!dx_TriggerTexture && dx_Sprite)
        dx_TriggerTexture = load_texture("Trigger.bmp", D3DCOLOR_XRGB(73, 125, 65));
    /*
    D3DXCreateFont creates a font object for a device and font.

    D3DXCreateFont(device, h, w, weight, miplevels, italic, charset, OutputPrecision, quality, pitchandfamily, pfaceanme, *ppfont)

    dx_Device (LPDIRECT3DDEVICE9) is the device we will be creating a font for.
    24 (INT) is the height of the characters in logical units.
    0 (UINT) is the width of the characters in logical units.
    FW_REGULAR (UNIT) is the typeface weight.
    0 (MipLevels) is the number of mipmap levels. MipMaps (not miplevels) are pre-calculated, optimized collections of images that accompany a main texture, intended to increase rendering speed and reduce aliasing artifacts
    false (BOOL) is if the italic font is true or not. In this case it is false.
    DEFAULT_CHARSET ( DWORD) is the character set of the font.
    OUT_CHARACTER_PRECIS (DWORD) specifies how Windows should attempt to match the desired font sizes and characteristics with actual fonts. In this case we are not using this feature.
    ANTIALIASED_QUALITY (DWORD) specifies how Windows should match the desired font with a real font. In this case we are always antialiasing if the font supports it and the size of the font is not too small or too large.
    DEFAULT_PITCH (DWORD) is the pitch and family index.
    Verdana (LPCTSTR) is the string containing the typeface name (font style).
    dx_Font (LPD3DXFONT*) returns a pointer to an ID3DXFont interface, representing the created font object.
    */
    D3DXCreateFont(dx_Device, 28, 0, FW_NORMAL, 2, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas", &dx_Font);

    return 0;

}

void Overlay::toggle_debug_ui() {
    debugUiMode = (DebugUiMode) ((((int) debugUiMode) + 1) % sizeof(DebugUiMode));
    switch (debugUiMode) {
        case DebugUiMode::FrameOnly:
            show_hint("Debug UI: Frame only");
            break;
        case DebugUiMode::Full:
            show_hint("Debug UI: Full");
            break;
        case DebugUiMode::TargetOnly:
            show_hint("Debug UI: Target only");
            break;
        case DebugUiMode::Off:
            show_hint("Debug UI: Off");
            break;
    }
}

void Overlay::toggle_ui() {
    uiMode = (UiMode) ((((int) uiMode) + 1) % sizeof(UiMode));
    switch (uiMode) {
        case UiMode::DebugOnly:
            show_hint("UI: Debug only");
            break;
        case UiMode::Full:
            show_hint("UI: Full");
            break;
        case UiMode::InfoOnly:
            show_hint("UI: Info only");
            break;
        case UiMode::Off:
            show_hint("UI: Off");
            break;
    }
}

void Overlay::render_debug_ui() {
    if ((debugUiMode == DebugUiMode::TargetOnly || debugUiMode == DebugUiMode::Full) && manager.enemyVisible) {
        //draw_box((float) manager.enemyCoords.x - 15, (float) manager.enemyCoords.y-15, 30, 30, 2, 128, 255, 0, 255);
        draw_circle((float) manager.enemyCoords.x + (float) manager.screenSize.x / 2, (float) manager.enemyCoords.y + (float) manager.screenSize.y / 2, 15, 128,
                    255, 0, 200);
    }
    if (debugUiMode == DebugUiMode::FrameOnly || debugUiMode == DebugUiMode::Full) {
        draw_box((float) manager.region.left, (float) manager.region.top, (float) manager.region.width, (float) manager.region.height, 2, 50, 50, 240, 200);
    }

}

void Overlay::render_ui() {
    if (uiMode != UiMode::InfoOnly && uiMode != UiMode::Full) return;
    dx_Sprite->Begin(D3DXSPRITE_ALPHABLEND);
    const int x = 5;
    const int y = 24;
    switch (manager.mode) {
        case flick:
            sprite_transform_draw(dx_FlickTexture, x, y, 50, 50, 0, 1, 0, 1.0f, D3DCOLOR_XRGB(255, 255, 255));
            break;
        case aim:
            sprite_transform_draw(dx_AimTexture, x, y, 50, 50, 0, 1, 0, 1.0f, D3DCOLOR_XRGB(255, 255, 255));
            break;
        case trigger:
            sprite_transform_draw(dx_TriggerTexture, x, y, 50, 50, 0, 1, 0, 1.0f, D3DCOLOR_XRGB(255, 255, 255));
            break;
        case hanzo:
            sprite_transform_draw(dx_HanzoTexture, x, y, 50, 50, 0, 1, 0, 1.0f, D3DCOLOR_XRGB(255, 255, 255));
            break;
    }
    dx_Sprite->End();
}

void Overlay::render_hints() {
    auto index = 0;
    for (auto &item: Overlay::hints.strings()) {
        draw_filled(60, (float) 24 + (float) 30 * index, (float) max(item.size() * 15, 180) + 5.0f, 30, 10, 10, 10, 190);
        draw_string((char *) item.c_str(), 65, 24 + 30 * index, 220, 200, 100); // Put Main procedure here like ESP etc.
        index++;
    }
}

int Overlay::render() {
    dx_Device->BeginScene();
    dx_Device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
    render_hints();
    if (uiMode == UiMode::Full || uiMode == UiMode::DebugOnly) {
        render_debug_ui();
    }
    if (uiMode == UiMode::Full || uiMode == UiMode::InfoOnly) {
        render_ui();
    }
    dx_Device->EndScene();
    dx_Device->PresentEx(0, 0, 0, 0, 0);
    return 0;
}

LRESULT CALLBACK Overlay::callback_proc(HWND pHwnd, UINT pMessage, WPARAM wParam, LPARAM lParam) {
    if (WM_NCCREATE == pMessage) {
        SetWindowLongPtr(pHwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT *) lParam)->lpCreateParams);
        return TRUE;
    }

    return ((Overlay *) GetWindowLongPtr(pHwnd, GWLP_USERDATA))->callback_proc_instance(pHwnd, pMessage, wParam, lParam);
}

LRESULT CALLBACK Overlay::callback_proc_instance(HWND pHWnd, UINT pMessage, WPARAM wParam, LPARAM lParam) {
    switch (pMessage) {
        case WM_PAINT: // we need to paint? lets paint!
            render();
            break;
        case WM_CREATE:
            return DwmExtendFrameIntoClientArea(pHWnd, &pMargin); // extension of window frame into client area
            break;
        case WM_DESTROY:
            PostQuitMessage(0); // We need to use this to exit a message loop
            break;
        default:
            break;
    }
    return DefWindowProc(pHWnd, pMessage, wParam, lParam); // Making sure all messages are processed
}

void Overlay::init(Manager &pManager) {
    if (sharedInstance != nullptr) return;
    sharedInstance = new Overlay(pManager);
    std::thread runThread(&Overlay::run, sharedInstance);
    runThread.detach();
    show_hint("MEMU3 - Version 1. Enjoy :)", 1000);
}

int WINAPI Overlay::run() {
    char overlayWindowName[] = "MEMU3-Overlay";
    WNDCLASSEXA OverlayWnd; // contains window class information
    OverlayWnd.cbSize = sizeof(WNDCLASSEXA); // size of struct, basically checking for version or check
    OverlayWnd.style = CS_HREDRAW | CS_VREDRAW;  // Style, redraw method type
    OverlayWnd.lpfnWndProc = callback_proc; // Pointer to the window procedure
    OverlayWnd.cbClsExtra = 0; // window class struct extra bytes
    OverlayWnd.cbWndExtra = 0; // window instance extra bytes
    OverlayWnd.hInstance = nullptr; // handle to the instance that contains the window procedure for the class
    OverlayWnd.hIcon = LoadIcon(NULL, IDI_APPLICATION); // basic window icon set
    OverlayWnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // basic window icon set
    OverlayWnd.hCursor = LoadCursor(NULL, IDC_ARROW); // basic window cursor icon set
    OverlayWnd.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(0, 0, 0)); // handle to the class background brush
    OverlayWnd.lpszMenuName = overlayWindowName;
    OverlayWnd.lpszClassName = overlayWindowName;

    // registers a window class for the use in call to this createwindowex func
    if (!RegisterClassExA(&OverlayWnd)) {
        return 1;
    }

    //TargetWnd = FindWindowA(0, manager.targetWindowName.c_str());

    /*
    CreateWindowEx creates an overlapped, pop-up, or child window with an extended window style.

    dwExStyle (DWORD) is the extended window style of the window being created.
    WS_EX_TOPMOST means that the window should be placed above all non-topmost windows and should stay above them, even when the window is deactivated
    WS_EX_LAYERED uses a layered window can significantly improve performance and visual effects for a window that has a complex shape, animates its shape, or wishes to use alpha blending effects.
    WS_EX_COMPOSITED paints all descendants of a window in bottom-to-top painting order using double-buffering.
    WS_EX_TRANSPARENT means that the window should not be painted until siblings beneath the window (that were created by the same thread) have been painted.

    lpClassName (LPCTSTR) is a null-terminated string or a class atom created by a previous call to the RegisterClass or RegisterClassEx function.
    lpWindowName (LPCSTR) is the window name.
    dwStyle (DWORD) is the style of the window being created.
    WS_POPUP means that the window is a pop-up window.
    x (int) is the horizontal position of the window.
    y (int) is the vertical position of the window.
    nWidth (int) is the width.
    nHeight (int) is the height.

    The last three nulls are all optional, and I wont bother mentioning them. If you are interested google CreateWindowEx.

    The dimensions for the overlay will be resized when the game is found.
    */


    //if (TargetWnd) {
    //GetWindowRect(TargetWnd, &WindowRect);
    //windowWidth = WindowRect.right - WindowRect.left;
    //windowHeight = WindowRect.bottom - WindowRect.top;
    hWnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, overlayWindowName, overlayWindowName, WS_POPUP, 1, 1,
                           manager.screenSize.x, manager.screenSize.y, 0, 0, 0, this);
    //}

    /*
    SetLayeredWindowAttributes sets the opacity and transparency color key for a layered window.
    */
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

    /*
    Show the layered window aka our overlay.
    */
    ShowWindow(hWnd, SW_SHOW);

    /*
    We use our handle to our overlay and initalize our D3D adapter.
    */
    if (init_d3d(hWnd) > 0) {
        return 1;
    }

    /*
    While we are not panicking, we will be enable our hack.
    */
    while (!manager.is_exit_requested()) {
        /*
        Dispatches incoming sent messages, checks the thread message queue for a posted message, and retrieves the message (if any exist). Messages are removed from the queue after processing due to PM_REMOVE.
        */
        if (PeekMessage(&Message, hWnd, 0, 0, PM_REMOVE)) {
            /*
            Translates virtual-key messages into character messages.
            */
            TranslateMessage(&Message);

            /*
            Dispatches a message to a window procedure.
            */
            DispatchMessage(&Message);
        }

        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    } // End of main Loop

    /*
    Lets exit immediately...
    */
    return 0;
}

Overlay::Overlay(Manager &pManager) : manager(pManager) {}

void Overlay::show_hint(std::string msg, int timeout) {
    if (sharedInstance == nullptr) return;
    hints.add(msg, timeout);
}

Overlay::~Overlay() {

}

