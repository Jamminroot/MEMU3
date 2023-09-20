#pragma once
#include "logger.h"
#include <string>
#include <iostream>
class ConsoleLogger : public Logger {
public:
    ConsoleLogger(){
        Logger::set_logger(this);
    }
    void log(const std::string &message,const int &duration = 0) const override {
        std::cout << message << std::endl;
    }
};