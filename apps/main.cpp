#include <iostream>
#include <nbody/renderer.hpp>

int main() {
    Renderer r(800, 600, "nbody");

    while(!r.shouldClose()) {
        r.beginFrame();

        r.endFrame();
    }

    return 0;
}