#pragma once

#include "logger.h"
#include "../Manager.h"
#include "../Overlay.h"
#include <string>
#include <iostream>
class OverlayLogger : public Logger {
public:
    OverlayLogger(Manager& manager){
        Overlay::init(manager);
        Logger::set_logger(this);
    }
    void log(const std::string &message,const int &duration = 0) const override {
        Overlay::show_hint(message, duration);
    }
};