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
#if WIN32 || !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
#else
    int len;
    int slength = (int) str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
#endif
}

std::string ws2s(const std::wstring &wstr) {

#if WIN32 || !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
#else
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
#endif
}