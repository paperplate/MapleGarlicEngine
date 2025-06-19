#include "MapleGarlicEngineWindow.hxx"

#include <iostream>

namespace MapleGarlicEngine {

static void error_callback(int code, const char* description){
    std::cerr << description << std::endl;
}

Window::Window(uint32_t w, uint32_t h, std::string name) : width(w), height(h), windowName(name) {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow() {
    glfwSetErrorCallback(error_callback);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) {
        std::cerr << "[" << __func__ << "]" << "GLFW init failed!" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "[" << __func__ << "]" << "window is null!" << std::endl;
    }
    glfwSetWindowUserPointer(window, this);
}

}
