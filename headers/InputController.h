#pragma once

#include "Manager.h"
#include <interception.h>

class InputController {
public:
    InputController(Manager &pManager);
    void move_by(const int &x, const int &y) const;
    void lmb_click() const;
private:
    Manager &manager;
    InterceptionContext context = interception_create_context();
    InterceptionDevice mouse;
    InterceptionDevice keyboard;
    void input_thread_handler();
    void handle_stroke(InterceptionStroke &stroke, InterceptionDevice &device);
    bool handle_keyboard_stroke(InterceptionKeyStroke &stroke, InterceptionDevice &device);
    bool handle_mouse_stroke(InterceptionMouseStroke &stroke, InterceptionDevice &device);
};