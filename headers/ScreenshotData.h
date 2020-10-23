#pragma once

#include <Windows.h>

class ScreenshotData
{
public:
	ScreenshotData(const int &pWidth, const int &pHeight);
	~ScreenshotData();
    RGBQUAD data[600*500];
    int size;
};