#include "Clove/Platform/Window.hpp"

namespace garlic::clove {
    Window::Window() = default;

    Window::~Window() = default;

    void Window::setVSync(bool enabled) {
        /*surface->setVSync(enabled);*/
    }

    bool Window::isVSync() const {
        return false;
        /*return surface->isVsync();*/
    }

    Keyboard &Window::getKeyboard() {
        return keyboard;
    }

    Mouse &Window::getMouse() {
        return mouse;
    }
}