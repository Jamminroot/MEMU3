#pragma once

#include <sstream>
#include <string>
#include <iosfwd>
#include <vector>

float clamp(float val, float min, float max);

float lerp(float t, float min, float max);

std::wstring s2ws(const std::string &str);

std::string ws2s(const std::wstring &wstr);

int next_random_user_delay();

std::string to_string(float a_value, const int n = 2);

std::string base64_encode(const std::string &in);

static std::vector<std::string> split_string(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}