#pragma once

#include <cstdint>

#include "MapleGarlicEngineWindow.hxx"
#include "Pipeline.hxx"

namespace MapleGarlicEngine {

class FirstApp {

    public:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    void run();

private:
    Window window{WIDTH, HEIGHT, "Hello Vulkan!"};
    Pipeline pipeline{"shaders/shader.vert.spv", "shaders/shader.frag.spv"};
};

}
