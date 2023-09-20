#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>
#include "../Coords.h"
#include "../Rect.h"

void print_bitmap_console(BITMAP &bitmap);

bool dump_bitmap(HBITMAP &hBitmap, LPCTSTR lpszFileName);

void print_pixel_console(unsigned char r, unsigned char g, unsigned char b);

void print_hbitmap_console(HBITMAP &hbitmap);

bool debug_print_layers(const std::vector<std::vector<Coords>> &layers, HBITMAP &hBitmap);

bool load_image_offset_region(const std::string &filename, const Rect &offset_region, HBITMAP &bitmap);