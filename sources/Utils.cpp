#include "../headers/Utils.h"
#include <rpc.h>
#include <random>
#include <chrono>
#include <filesystem>
namespace fs = std::filesystem;

#if  !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
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

float lerp_value(float t, float min, float max) {
    return min + t * (max - min);
}

std::wstring s2ws(const std::string &str) {
#if !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
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

#if !(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING || _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
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

std::string to_string(double a_value, const int n) {
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

std::vector<std::string> split_string(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string hashtable_name(const std::vector<RGBQUAD> &pColors) {
    std::string bytes;
    for (auto targetColor: pColors) {
        bytes.push_back(targetColor.rgbRed);
        bytes.push_back(targetColor.rgbGreen);
        bytes.push_back(targetColor.rgbBlue);
        bytes.push_back(targetColor.rgbReserved);
    }
    return "ct_" + base64_encode(bytes) + ".bin";
}

std::vector<std::string> list_files_by_mask(const std::string &mask, const std::string &path) {
    std::vector<std::string> configs = std::vector<std::string>();
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().filename().string().find(mask) != std::string::npos) {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

BITMAPINFO create_bitmap_info_struct(int width, int height, int bitCount) {
    BITMAPINFO bmpInfo = {0};
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = width;
    bmpInfo.bmiHeader.biHeight = height;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = bitCount;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    return bmpInfo;
}

std::string get_last_error(DWORD err) {
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    //Free the buffer.
    LocalFree(messageBuffer);
    return message;
}
