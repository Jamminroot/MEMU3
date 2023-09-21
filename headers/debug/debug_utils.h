#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>
#include "../Coords.h"
#include "../Rect.h"

void print_bitmap_console(BITMAP &bitmap, int scale_x = 1, int scale_y = 3);

bool dump_bitmap(HBITMAP &hBitmap, const std::string &filename);

void print_pixel_console(unsigned char r, unsigned char g, unsigned char b);

void print_hbitmap_console(HBITMAP &hbitmap, int scale_x = 1, int scale_y = 3);

bool debug_print_layers(const std::vector<std::vector<Coords>> &layers, HBITMAP &p_hbitmap, const std::vector<COLORREF> &colors);

bool debug_print_layer(const std::vector<Coords> &layer, HBITMAP &p_hbitmap, const COLORREF &color);

bool load_image_offset_region(const std::string &filename, const Rect &offset_region, HBITMAP &bitmap);

void debug_print_grey_background(HBITMAP &p_hdc, const HBITMAP &p_hbitmap, double d);