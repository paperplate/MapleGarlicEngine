#include "app.hxx"


#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
  MapleGarlicEngine::FirstApp app{};
  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << "[" << __func__ << "]" << e.what() << std::endl;
    //std::cerr << std::stacktrace::current() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
