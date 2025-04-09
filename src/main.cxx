#include <exception>
#include <cstdlib>
#include <iostream>
//#include <stacktrace>
#include "helloTriangle.hxx"
//import MapleGarlicEngine;

/*extern "C" int myMain() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[" << __func__ << "]" << e.what() << std::endl;
  //      std::cerr << std::stacktrace::current() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}*/

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[" << __func__ << "]" << e.what() << std::endl;
  //      std::cerr << std::stacktrace::current() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
