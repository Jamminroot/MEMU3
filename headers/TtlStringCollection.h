#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

class TtlString {
public:
    unsigned int expirationTimePoint = 0;
    std::string message;
};

class TtlStringCollection {
public:
    TtlStringCollection(const int pMilliseconds);
    void add(std::string &msg, int timeout);
    std::vector<std::string> strings();
private:
    [[noreturn]] void vector_cleaner(int);
    std::vector<TtlString> data = std::vector<TtlString>();
};
