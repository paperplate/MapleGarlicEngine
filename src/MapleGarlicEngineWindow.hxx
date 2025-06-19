#pragma once

#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>


namespace MapleGarlicEngine {


class Window {

public:
    Window(uint32_t w, uint32_t h, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window &operator=(const Window&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window); }

private:
    GLFWwindow *window;
    const uint32_t width;
    const uint32_t height;
    std::string windowName;


    void initWindow();

};

}
