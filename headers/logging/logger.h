#pragma once
#include <string>

class Logger {
public:
    Logger() = default;
    virtual ~Logger() = default;

    virtual void log(const std::string &message, const int &duration = 5000) const = 0;

    void operator << (const std::string &message) const {
        // Check if message is null or whitespace
        if (message.find_first_not_of(' ') == std::string::npos) {
            return;
        }
        log(message);
    }

    static void set_logger(Logger *logger) {
        s_logger = logger;
    }

    static void show(const std::string &message, const int &duration = 5000) {
        if (s_logger == nullptr) {
            return;
        }
        s_logger->log(message, duration);
    }
private:
    inline static Logger* s_logger;
};