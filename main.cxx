#include <exception>
#include <cstdlib>
#include <iostream>

import MapleGarlicEngine;

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[" << __func__ << "]" << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
