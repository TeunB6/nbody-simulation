#include <iostream>
#include <nbody/renderer.hpp>
#include <nbody/simulator.hpp>

int main() {
    Renderer r(800, 600, "nbody");
    Simulator sim(1000);
    const float h = 1.f / 120.f;
    float acc = 0.f;

    while(!r.shouldClose()) {
        float dt = 1.f / 60.f;
        acc += std::min(dt, 0.25f);          // clamp so a stall doesn't cause a spiral
        while (acc >= h) { sim.step(h); acc -= h; }
        r.beginFrame();
        r.draw(sim.positions(), sim.count());
        r.endFrame();
    }

    return 0;
}