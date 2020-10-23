#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <atomic>

class TtlString {
public:
    int id;
    unsigned int expirationTimePoint = 0;
    std::string message;
};

class TtlStringCollection {
public:
    TtlStringCollection(const int pMilliseconds, void (* pCallback) (int));

    int add(std::string msg, int timeout);

    std::vector<std::string> strings();

private:
    void (* handler) (int);
    std::atomic_int nextId = 0;
    void vector_cleaner(int);
    std::vector<TtlString> data = std::vector<TtlString>();
};
