#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>

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
    bool empty();
private:
    std::mutex empty_mutex;
    std::condition_variable empty_cond;
    [[noreturn]] void vector_cleaner(int);
    std::vector<TtlString> data = std::vector<TtlString>();
};
