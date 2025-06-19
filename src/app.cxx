#include "app.hxx"

namespace MapleGarlicEngine {

void FirstApp::run() {
    while (!window.shouldClose()) {
        glfwPollEvents();
    }
}

}
