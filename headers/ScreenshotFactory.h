#pragma once
#include "ScreenshotData.h"
#include "Manager.h"

class ScreenshotFactory
{
public:
	explicit ScreenshotFactory(Manager& pManager);
	bool update_screenshot();
private:
	Manager& manager;
	static void release(HDC& hdc, HDC& captureDC, HBITMAP& hBmp);
};

