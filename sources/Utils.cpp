#include "../headers/Utils.h"
#include <rpc.h>
#include <random>
#include <chrono>

#if  !(WIN32 || !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS))
#include <locale>
#include <codecvt>
#endif

std::default_random_engine generator((int)std::chrono::system_clock::now().time_since_epoch().count()%10000);
std::uniform_int_distribution<int> random_user_delay(9,25);

int next_random_user_delay() {
    return random_user_delay(generator);
}

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

std::string to_string(float a_value, const int n) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}
std::string base64_encode(const std::string &in) {
    std::string out;

    int val=0, valb=-6;
    for (auto c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}
