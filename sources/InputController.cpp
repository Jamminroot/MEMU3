#include "../headers/InputController.h"
#include "../headers/Overlay.h"
#include "../headers/Utils.h"
#include <thread>
#include <iostream>

#if (TARGET_64)
#pragma comment(lib, "../Interception/x64/interception.lib")
#else
#pragma  comment(lib, "Interception/x86/interception.lib")
#endif

const int AimMouseDownKeys = InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_5_DOWN | InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_4_DOWN |
                             InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;

InputController::InputController(Manager &pManager) : manager(pManager) {

    context = interception_create_context();
    std::thread inputFiltrationThread(&InputController::input_thread_handler, this);
    inputFiltrationThread.detach();
}

void InputController::input_thread_handler() {
    auto mouse_captured = false;
    auto keyboard_captured = false;

    InterceptionDevice device;
    InterceptionStroke stroke;
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);

    while ((!mouse_captured || !keyboard_captured) && interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        if (interception_is_mouse(device) && !mouse_captured) {
            std::cout << "Mouse device initialized.\n";
            mouse_captured = true;
            mouse = device;
        }
        if (interception_is_keyboard(device) && !keyboard_captured) {
            std::cout << "Keyboard device initialized.\n";
            keyboard_captured = true;
            keyboard = device;
        }
        handle_stroke(stroke, device);
    }

    while (!manager.is_exit_requested() && interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        handle_stroke(stroke, device);
    }
    interception_destroy_context(context);
}

void InputController::handle_stroke(InterceptionStroke &pStroke, InterceptionDevice &device) {
    auto skip = false;
    if (interception_is_mouse(device)) {
        InterceptionMouseStroke &mouseStroke = *(InterceptionMouseStroke *) &pStroke;
        skip = handle_mouse_stroke(mouseStroke, device);
    }

    if (interception_is_keyboard(device)) {
        InterceptionKeyStroke &keyStroke = *(InterceptionKeyStroke *) &pStroke;
        skip = handle_keyboard_stroke(keyStroke, device);
    }
    if (!skip) {
        interception_send(context, device, &pStroke, 1);
    }
}

bool InputController::handle_keyboard_stroke(InterceptionKeyStroke &stroke, InterceptionDevice &device) {
    switch (stroke.code) {
        case KeyCode::RightAlt:
            manager.set_running(!manager.is_running());
            break;
        case KeyCode::F2:
            manager.toggle_mode();
            break;
        case KeyCode::Numpad0:
            Overlay::toggle_ui();
            break;
        case KeyCode::NumpadDelete:
            Overlay::toggle_debug_ui();
            break;
        case KeyCode::NumpadMinus:
            manager.decrease_aim_strength();
            break;
        case KeyCode::NumpadPlus:
            manager.increase_aim_strength();
            break;
        case KeyCode::PageUp:
            manager.increase_sensitivity();
            break;
        case KeyCode::PageDown:
            manager.decrease_sensitivity();
            break;
    }
    return false;
}

bool InputController::handle_mouse_stroke(InterceptionMouseStroke &stroke, InterceptionDevice &device) {
    auto cache = manager.mouseTriggerKeyStates;
    manager.mouseTriggerKeyStates |= stroke.state & AimMouseDownKeys;

    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN : 0;
    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_5_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_5_DOWN : 0;
    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_4_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_4_DOWN : 0;

    manager.triggerStateChanged = manager.mouseTriggerKeyStates != cache;

    switch (manager.mode) {
        case flick: {
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN) != 0) manager.flickReady = true;
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_RIGHT_BUTTON_UP) != 0) manager.flickReady = false;
            break;
        }
        case hanzo: {
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN) != 0) manager.flickReady = true;
            break;
        }
    }
    manager.mouseTriggered = (manager.mouseTriggerKeyStates & AimMouseDownKeys) > 0;
    return manager.flickReady && manager.triggerStateChanged && (manager.mode == flick || manager.mode == hanzo) && manager.enemyVisible &&
           (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN) != 0;
}

void InputController::move_by(const int &x, const int &y) const {
    InterceptionMouseStroke mstroke = InterceptionMouseStroke();
    mstroke.x = x;
    mstroke.y = y;
    mstroke.flags = InterceptionMouseFlag::INTERCEPTION_MOUSE_MOVE_RELATIVE;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
}

void InputController::lmb_click() const {
    InterceptionMouseStroke mstroke = InterceptionMouseStroke();
    mstroke.state = InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay()));
    mstroke.state = InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
}
