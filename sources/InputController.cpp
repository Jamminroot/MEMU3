#include "../headers/InputController.h"
#include "../headers/Overlay.h"
#include "../headers/Utils.h"
#include <thread>

#include <iostream>

#if defined(TARGET_64) || defined(_WIN64)

#if defined(CMAKELISTS)
#pragma comment(lib, "../Interception/x64/interception.lib")
#else
#pragma comment(lib, "Interception/x64/interception.lib")
#endif //defined(CMAKELISTS)

#else

#if defined(CMAKELISTS)
#pragma  comment(lib, "../Interception/x86/interception.lib")
#else
#pragma  comment(lib, "Interception/x86/interception.lib")
#endif //defined(CMAKELISTS)

#endif //defined(TARGET_64) || defined(_WIN64)

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
#if DEBUG || _DEBUG
            Overlay::show_hint("Mouse device initialized");
#endif
            mouse_captured = true;
            mouse = device;
        }
        if (interception_is_keyboard(device) && !keyboard_captured) {
#if DEBUG || _DEBUG
            Overlay::show_hint("Keyboard device initialized");
#endif
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
        if ((mouseStroke.flags & InterceptionMouseFlag::INTERCEPTION_MOUSE_CUSTOM) == 0) {
            skip = handle_mouse_stroke(mouseStroke);
        }
    }

    if (interception_is_keyboard(device)) {
        InterceptionKeyStroke &keyStroke = *(InterceptionKeyStroke *) &pStroke;
        skip = handle_keyboard_stroke(keyStroke);
    }
    if (!skip) {
        interception_send(context, device, &pStroke, 1);
    }
}

bool InputController::handle_keyboard_stroke(InterceptionKeyStroke &stroke) {
    switch (stroke.code) {
        case KeyCode::RightAlt:
            manager.set_running(!manager.is_running());
            break;
        case KeyCode::F2:
            manager.toggle_mode();
            break;
        case KeyCode::NumLock:
        case KeyCode::End:
        //case KeyCode::Backspace:
            manager.request_exit();
            break;
        case KeyCode::Numpad0:
            Overlay::toggle_ui();
            break;
        case KeyCode::NumpadDelete:
            Overlay::toggle_debug_ui();
            break;
        case KeyCode::NumpadMinus:
        case KeyCode::DashUnderscore:
            manager.decrease_mode_value();
            break;
        case KeyCode::NumpadPlus:
        case KeyCode::PlusEquals:
            manager.increase_mode_value();
            break;
        case KeyCode::Numpad9:
            manager.increase_sensitivity();
            break;
        case KeyCode::Numpad3:
            manager.decrease_sensitivity();
            break;
        case KeyCode::Numpad8:
            manager.toggle_next_colorconfig();
            break;
        case KeyCode::NumpadDivide:
            manager.toggle_next_strengthmap();
            break;
        default:
            std::cout << "Unknown keycode: "<< stroke.code << std::endl;
    }
    return false;
}

bool InputController::handle_mouse_stroke(InterceptionMouseStroke &stroke) {
    auto cache = manager.mouseTriggerKeyStates;
    manager.mouseTriggerKeyStates |= stroke.state & AimMouseDownKeys;

    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN : 0;
    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_5_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_5_DOWN : 0;
    manager.mouseTriggerKeyStates ^=
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_4_UP) != 0 ? InterceptionMouseState::INTERCEPTION_MOUSE_BUTTON_4_DOWN : 0;

    manager.triggered = manager.mouseTriggerKeyStates != cache;

    bool hanzoSkip = false;
    switch (manager.mode) {
        case flick: {
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN) != 0) {
                manager.flickReady = true;
            }
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_RIGHT_BUTTON_UP) != 0) {
                manager.flickReady = false;
            }
            break;
        }
        case hanzo: {
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN) != 0) { manager.flickReady = true; }
            if ((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP) != 0 && manager.enemyVisible) {
                manager.triggered = true;
                hanzoSkip = true;
            }
            break;
        }
        default: break;
    }
    if (manager.mode == flick && (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP) != 0){
        manager.readyForNextFlick = true;
    }
    if (manager.mode != hanzo) {
        manager.triggered = (manager.mouseTriggerKeyStates & AimMouseDownKeys) > 0;
    }
    /* return manager.flickReady && manager.mouseKeyState && (manager.mode == flick || manager.mode == hanzo) && manager.enemyVisible &&
            (stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN) != 0;*/
    return hanzoSkip || manager.flickReady && manager.triggered && manager.enemyVisible &&
                        ((((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN) != 0) && manager.mode == flick) ||
                         (((stroke.state & InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP) != 0) && manager.mode == hanzo));
}

void InputController::move_by(const int &x, const int &y) const {
    InterceptionMouseStroke mstroke = InterceptionMouseStroke();
    mstroke.x = x;
    mstroke.y = y;
    mstroke.flags = (InterceptionMouseFlag::INTERCEPTION_MOUSE_MOVE_RELATIVE | InterceptionMouseFlag::INTERCEPTION_MOUSE_CUSTOM);
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
}

void InputController::lmb_click() const {
    InterceptionMouseStroke mstroke = InterceptionMouseStroke();
    mstroke.flags = InterceptionMouseFlag::INTERCEPTION_MOUSE_CUSTOM;
    mstroke.state = InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(next_random_user_delay()));
    mstroke.state = InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
}

void InputController::lmb_release() const {
    InterceptionMouseStroke mstroke = InterceptionMouseStroke();
    mstroke.flags = InterceptionMouseFlag::INTERCEPTION_MOUSE_CUSTOM;
    mstroke.state = InterceptionMouseState::INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
    interception_send(context, mouse, (InterceptionStroke *) &mstroke, 1);
}
