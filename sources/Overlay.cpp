#pragma once

#include "../headers/Overlay.h"

#include <algorithm>
#include <dwmapi.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

IDirect3D9Ex *dx_Object = NULL;
IDirect3DDevice9Ex *dx_Device = NULL;
D3DPRESENT_PARAMETERS dx_Params;
ID3DXLine *dx_Line;
ID3DXFont *dx_Font = 0;

HWND hWnd, TargetWnd;
MSG Message;
RECT WindowRect, ClientRect;
int windowWidth, windowHeight;
int clientWidth = 1600, clientHeight = 900;
int borderWidth, borderHeight;

char lWindowName[256] = "Overlay";
char tWindowName[256] = "Untitled - Paint"; // put Game window name here

const MARGINS pMargin = {0, 0, clientWidth, clientHeight};

int Overlay::DrawString(char *String, int x, int y, int r, int g, int b, ID3DXFont *ifont) {
    RECT ShadowPos;
    ShadowPos.left = x + 1;
    ShadowPos.top = y + 1;
    RECT FontPos;
    FontPos.left = x;
    FontPos.top = y;
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
    return 0;
}

int Overlay::DrawShadowString(char *String, int x, int y, int r, int g, int b, ID3DXFont *ifont) {
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
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &Fonts3, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &Fonts2, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &Fonts1, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &Fonts, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
    ifont->DrawTextA(nullptr, String, (int) strlen(String), &Font, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
    return 0;
}

void Overlay::GradientFunc(int x, int y, int w, int h, int r, int g, int b, int a) {
    int iColorr, iColorg, iColorb;
    for (int i = 1; i < h; i++) {
        iColorr = (int) ((float) i / h * r);
        iColorg = (int) ((float) i / h * g);
        iColorb = (int) ((float) i / h * b);
        DrawFilled((float) x, (float) y + i, (float) w, (float) 1, r - iColorr, g - iColorg, b - iColorb, a);
    }
}

void Overlay::DrawLine(float x, float y, float xx, float yy, int r, int g, int b, int a) {
    D3DXVECTOR2 dLine[2];

    dx_Line->SetWidth(1);

    dLine[0].x = x;
    dLine[0].y = y;

    dLine[1].x = xx;
    dLine[1].y = yy;

    dx_Line->Draw(dLine, 2, D3DCOLOR_ARGB(a, r, g, b));

}

void Overlay::DrawFilled(float x, float y, float w, float h, int r, int g, int b, int a) {
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

void Overlay::DrawBox(float x, float y, float width, float height, float px, int r, int g, int b, int a) {
    D3DXVECTOR2 points[5];
    points[0] = D3DXVECTOR2(x, y);
    points[1] = D3DXVECTOR2(x + width, y);
    points[2] = D3DXVECTOR2(x + width, y + height);
    points[3] = D3DXVECTOR2(x, y + height);
    points[4] = D3DXVECTOR2(x, y);
    dx_Line->SetWidth(px);
    dx_Line->Draw(points, 5, D3DCOLOR_RGBA(r, g, b, a));
}

void Overlay::DrawGUIBox(float x, float y, float w, float h, int r, int g, int b, int a, int rr, int gg, int bb, int aa) {
    DrawBox(x, y, w, h, 1, r, g, b, a);
    DrawFilled(x, y, w, h, rr, gg, bb, a);
}

void Overlay::DrawHealthBar(float x, float y, float w, float h, int r, int g, int b, int a) {
    DrawFilled(x, y, w, h, r, g, b, a);
}

void Overlay::DrawHealthBarBack(float x, float y, float w, float h, int a) {
    DrawFilled(x, y, w, h, 0, 0, 0, a);
}

void Overlay::DrawCenterLine(float x, float y, int width, int r, int g, int b) {
    D3DXVECTOR2 dPoints[2];
    dPoints[0] = D3DXVECTOR2(x, y);
    dPoints[1] = D3DXVECTOR2((float) windowWidth / 2, (float) windowHeight);
    dx_Line->SetWidth((float) width);
    dx_Line->Draw(dPoints, 2, D3DCOLOR_RGBA(r, g, b, 255));
}

/*
We require to initialize the D3D drawing, so we require hWnd. Windows identifies each form or application by assigning it a handle or also known as hWnd.
*/
int Overlay::D3D9Init(HWND hWnd) {
    // We get our Process Access and Module Bases


    /*
    We need to check to see if we can create an IDirect3D9Ex object and return an interface to it. Why is D3D_SDK_VERSION passed? Because we will need to ensure that the header files used in the compiled application match the version of the installed runtime DLLs. Why are we passing the object to dx_Object? Because we are creating an IDirect3D9Ex object, and we need to store it somewhere. If it fails, the app crashes (the DLL), and if it passes, it continues, simple huh?
    */
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &dx_Object))) {
        exit(1);
    }

    /*
    We created the dx_Param earlier, it is a D3DPRESENT_PARAMETERS structure. It contains many variables you can modify but in this source we are only modifying these variables.

    BackBufferFormat (D3DFORMAT) is the buffer that is drawn off-screen and will be switched with the front buffer at the next frame. This is considered double buffering, which is what you need to do in GDI to ensure that it does not flicker. But GDI will still flicker because it is "slow" you could persay.

    D3DFMT_A8R8G8B8	(Value: 21) is an 32-bit ARGB pixel format with alpha, using 8 bits per channel.
    */

    dx_Params.BackBufferFormat = D3DFMT_A8R8G8B8;

    dx_Params.BackBufferWidth = windowWidth;
    dx_Params.BackBufferHeight = windowHeight;
    dx_Params.EnableAutoDepthStencil = TRUE;
    dx_Params.AutoDepthStencilFormat = D3DFMT_D16;

    /*
    hDeviceWindow (HWND) is the form or application that determines the location and size of the back buffer on the screen.
    */
    dx_Params.hDeviceWindow = hWnd;

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
        exit(1);
    }

    if (!dx_Line)
        D3DXCreateLine(dx_Device, &dx_Line);

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
    D3DXCreateFont(dx_Device, 18, 0, FW_LIGHT, 1, false, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial", &dx_Font);

    return 0;

}

int Overlay::Render() {
    dx_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);

    dx_Device->BeginScene();

    if (TargetWnd == GetForegroundWindow()) {
        DrawString((char *) "Test", windowWidth / 2, windowHeight / 2, 255, 0, 0, dx_Font); // Put Main procedure here like ESP etc.
    }

    Sleep(100);
    dx_Device->EndScene();
    dx_Device->PresentEx(0, 0, 0, 0, 0);

    return 0;
}

LRESULT CALLBACK Overlay::Proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    if (WM_NCCREATE == Message) {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT *) lParam)->lpCreateParams);
        return TRUE;
    }

    return ((Overlay *) GetWindowLongPtr(hWnd, GWLP_USERDATA))->_Proc(hWnd, Message, wParam, lParam);
}

LRESULT CALLBACK Overlay::_Proc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
        case WM_PAINT: // we need to paint? lets paint!
            Render();
            break;
        case WM_CREATE:
            return DwmExtendFrameIntoClientArea(hWnd, &pMargin); // extension of window frame into client area
            break;
        case WM_DESTROY:
            PostQuitMessage(0); // We need to use this to exit a message loop
            break;
        default:
            break;
    }
    return DefWindowProc(hWnd, Message, wParam, lParam); // Making sure all messages are processed
}

void Overlay::Init(HINSTANCE hInstance, Manager &pManager) {
    if (sharedInstance != nullptr) return;
    sharedInstance = new Overlay(pManager);
    sharedInstance->Run(hInstance);
}

int WINAPI Overlay::Run(HINSTANCE hInstance) {
    WNDCLASSEXA OverlayWnd; // contains window class information
    OverlayWnd.cbSize = sizeof(WNDCLASSEXA); // size of struct, basically checking for version or check
    OverlayWnd.style = CS_HREDRAW | CS_VREDRAW;  // Style, redraw method type
    OverlayWnd.lpfnWndProc = Proc; // Pointer to the window procedure
    OverlayWnd.cbClsExtra = 0; // window class struct extra bytes
    OverlayWnd.cbWndExtra = 0; // window instance extra bytes
    OverlayWnd.hInstance = hInstance; // handle to the instance that contains the window procedure for the class
    OverlayWnd.hIcon = LoadIcon(NULL, IDI_APPLICATION); // basic window icon set
    OverlayWnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // basic window icon set
    OverlayWnd.hCursor = LoadCursor(NULL, IDC_ARROW); // basic window cursor icon set
    OverlayWnd.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(0, 0, 0)); // handle to the class background brush
    OverlayWnd.lpszMenuName = lWindowName;
    OverlayWnd.lpszClassName = lWindowName;

    // registers a window class for the use in call to this createwindowex func
    if (!RegisterClassExA(&OverlayWnd)) {
        exit(1);
    }

    TargetWnd = FindWindowA(0, tWindowName);

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


    if (TargetWnd) {
        GetWindowRect(TargetWnd, &WindowRect);
        windowWidth = WindowRect.right - WindowRect.left;
        windowHeight = WindowRect.bottom - WindowRect.top;
        hWnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, lWindowName, lWindowName, WS_POPUP, 1, 1, windowWidth, windowHeight, 0, 0, 0,
                               this);
    }


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
    D3D9Init(hWnd);

    bool panic = false;

    /*
    While we are not panicking, we will be enable our hack.
    */
    while (panic == false) {
        Sleep(100);
        if (GetAsyncKeyState(VK_F12))
            panic = true;

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

        /*
        Set the hWnd for the game we want.
        */
        TargetWnd = FindWindowA(0, tWindowName);

        /*
        If there is no game, quit.
        */
        if (!TargetWnd)
            exit(0);

        /*
        Set the RECT using the targeted window.
        */
        GetWindowRect(TargetWnd, &WindowRect);
        GetClientRect(TargetWnd, &ClientRect);

        windowWidth = WindowRect.right - WindowRect.left;
        windowHeight = WindowRect.bottom - WindowRect.top;

        /*
        Resize the overlay matching what is required.
        */
        DWORD dwStyle = GetWindowLong(TargetWnd, GWL_STYLE);

        if (dwStyle & WS_BORDER) {
            windowHeight = WindowRect.bottom - WindowRect.top;
            windowWidth = WindowRect.right - WindowRect.left;
            clientHeight = ClientRect.bottom - ClientRect.top;
            clientWidth = ClientRect.right - ClientRect.left;
            borderHeight = (windowHeight - ClientRect.bottom);
            borderWidth = (windowWidth - ClientRect.right) / 2; //only want one side
            borderHeight -= borderWidth; //remove bottom from width, bottom is the same size as either side so we subtract that

            WindowRect.left += borderWidth;
            WindowRect.top += borderHeight;
        }

        MoveWindow(hWnd, WindowRect.left, WindowRect.top, clientWidth, clientHeight, true);

    } // End of Panic Loop

    /*
    Lets exit immediately...
    */
    exit(0);

}

Overlay::Overlay(Manager &pManager) : manager(pManager) {

}
