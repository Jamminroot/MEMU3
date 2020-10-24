#pragma once

#include <Windows.h>
#include <dwmapi.h>

HWND hWnd, TargetWnd;
MSG Message;
RECT WindowRect, ClientRect;
int windowWidth, windowHeight;
int clientWidth = 1600, clientHeight = 900;
int borderWidth, borderHeight;


char lWindowName[256] = "Overlay";
char tWindowName[256] = "Untitled - Paint"; // put Game window name here

const MARGINS pMargin = { 0,0, clientWidth, clientHeight };




