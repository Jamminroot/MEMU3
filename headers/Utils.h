#pragma once

#include <sstream>
#include <string>
#include <iosfwd>
#include <vector>
#include <windows.h>

float clamp(float val, float min, float max);

float lerp_value(float t, float min, float max);

std::wstring s2ws(const std::string &str);

std::string ws2s(const std::wstring &wstr);

int next_random_user_delay();

std::string to_string(float a_value, const int n = 2);

std::string to_string(double a_value, const int n = 2);

std::string base64_encode(const std::string &in);

std::vector<std::string> split_string(const std::string& s, char delimiter);

std::string hashtable_name(const std::vector<RGBQUAD> &pColors);

std::vector<std::string> list_files_by_mask(const std::string &mask, const std::string &path = ".");

BITMAPINFO create_bitmap_info_struct(int width, int height, int bitCount = 32);

std::string get_last_error(DWORD err);