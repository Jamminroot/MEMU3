#include "../headers/Utils.h"
#include <rpc.h>
#include <locale>
#include <codecvt>

float clamp(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

std::wstring s2ws(const std::string &str) {
    int len;
    int slength = (int) str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

std::string ws2s(const std::wstring &wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}