#pragma once

#include <string>

float clamp(float val, float min, float max);

float lerp(float t, float min, float max);

std::wstring s2ws(const std::string &str);

std::string ws2s(const std::wstring &wstr);

int next_random_user_delay();